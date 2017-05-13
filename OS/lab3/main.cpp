/* Author: Zhe Xu
 * Course: Operating Systems Fall16
 * Assignment: Lab3 virtual memory management
 * Date: 11/02/2016
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include "replacement.h"
#include "pte.h"

using namespace std;

// printing options
struct OPTION{
    bool O, P, F, S;
} op = {false, false, false, false};

// paging statistics
struct STATISTICS{
    int inst, unmaps, maps, ins, outs, zeros;
} stats = {0, 0, 0, 0, 0, 0};

PTE page_table[64] = {{0}};                       // page table
vector<unsigned int> frame_table;               // frame table
vector<unsigned int> inverse_table(64, -1);     // inverse table

/* Function: readCommand
 * reads user options and returns the page replacement algorithm required.
 * argc: the number of command line arguments.
 * argv: the argument strings.
 * random_file: a pointer to the file containing random number.
 * returns: a pointer to the replacement class.
 */

replacement* readCommand(int argc, char* argv[], FILE* random_file){
    int c;
    int fsize = 0;
    char temp[10];
    replacement* r;
    while((c = getopt(argc, argv, "a:o:f:"))!= -1){
        switch(c){
            case 'a':
                switch(optarg[0]){
                    case 'N':r = new NRU(random_file);break;
                    case 'l':r = new LRU();break;
                    case 'r':r = new Random(random_file);break;
                    case 'f':r = new FIFO();break;
                    case 's':r = new Second();break;
                    case 'c':r = new ClockP();break;
                    case 'X':r =  new ClockV();break;
                    case 'a':r = new AgingP();break;
                    case 'Y':r = new AgingV();break;
                    default:break;
                }
                break;
            case 'o':{
                for(unsigned int i = 0; i < strlen(optarg); i++){
                    switch(optarg[i]){
                        case 'O': op.O = true; break;
                        case 'P': op.P = true; break;
                        case 'F': op.F = true; break;
                        case 'S': op.S = true; break;
                        default:break;
                    }
                }
                break;
            }
            case 'f':{
                strncpy(temp, optarg, strlen(optarg));
                sscanf(temp, "%d", &fsize);
            }
            break;
            default:break;
        }
    }
    r->frame_size = fsize;
    return r;
}

/* Function: print_virtual_page
 * prints out each virtual page with required specifics.
 */
void print_virtual_page(){
    for(unsigned int i = 0; i < 64; i++){
        if(getPresent(page_table[i]) == 1) {
            printf("%d:", i);
            if(getReferenced(page_table[i]) == 1) {
                printf("R");
            }
            else printf("-");

            if(getModified(page_table[i]) == 1) {
                printf("M");
            }
            else printf("-");

            if(getPageout(page_table[i]) == 1) {
                printf("S ");
            }
            else printf("- ");
        }
        else {
            if(getPageout(page_table[i]) == 1) {
                printf("# ");
            }
            else printf("* ");
        }
    }
    printf("\n");
}

/* Function: print_frame
 * prints out each frame with required specifics.
 */
void print_frame(){
    for(unsigned int i = 0; i < inverse_table.size(); ++i) {
        if(inverse_table[i] == (unsigned int)-1) continue;
        else printf("%d ", inverse_table[i]);
    }
    printf("\n");
}

/* Function: print_statistics
 * prints out paging statistics as required by the assignment.
 */
void print_statistics(){
    unsigned long long cost;

    cost = (long long)stats.inst + 400 * (long long)(stats.maps
                + stats.unmaps) + 3000 * (long long)(stats.ins + stats.outs)
                + 150 * (long long)stats.zeros;

    printf("SUM %d U=%d M=%d I=%d O=%d Z=%d ===> %llu\n",
                stats.inst, stats.unmaps, stats.maps, stats.ins,
                stats.outs, stats.zeros, cost);
}

/* Function: print_all
 * prints out page information, frame information and paging statistics as requested by user options.
 */
void print_all(){
    if(op.P) print_virtual_page();
    if(op.F) print_frame();
    if(op.S) print_statistics();
}


/* Function: simulation
 * runs the memory management simulation.
 * input_file: a pointer to the file containing page request instructions.
 * r: a pointer to the replacement class.
 */
 void simulation(FILE* input_file, replacement* r){
    char *buffer = NULL;
    size_t len;
    int mode;
    unsigned int number;

	while(getline(&buffer, &len, input_file) != -1){
        if(buffer[0]!='#')
            sscanf(buffer, "%d%d", &mode, &number);
        else continue;

        if(op.O) printf("==> inst: %d %d\n", mode, number);

        if(getPresent(page_table[number]) == 1){
            r->usage(frame_table, getPFN(page_table[number]));
            setReferenced(page_table[number],1);
            if(mode == 1) setModified(page_table[number], 1);
        }
        else{
            unsigned int frame_number;
            if(frame_table.size() < r->frame_size){
                frame_number = frame_table.size();
                setPFN(page_table[number], frame_number);
                setPresent(page_table[number], 1);
                if(op.O){
                    printf("%d: ZERO     %4d\n", stats.inst, frame_number);
                    printf("%d: MAP  %4d%4d\n", stats.inst, number, frame_number);
                }
                stats.maps++;
                stats.zeros++;
                frame_table.push_back(frame_number);
                inverse_table[frame_number] = number;
            }
            else{
                stats.unmaps++;
                frame_number = r->selectPage(page_table, frame_table, inverse_table);

                if(op.O){
                printf("%d: UNMAP%4d%4d\n", stats.inst, inverse_table[frame_number], frame_number);
                }

                if(getModified(page_table[inverse_table[frame_number]]) == 1){
                    setModified(page_table[inverse_table[frame_number]], 0);
                    setPageout(page_table[inverse_table[frame_number]], 1);
                    if(op.O){
                        printf("%d: OUT  %4d%4d\n", stats.inst, inverse_table[frame_number], frame_number);
                    }
                    stats.outs++;
                }
                r->unmappage(page_table[inverse_table[frame_number]], frame_table, inverse_table);

                if(getPageout(page_table[number]) == 1){
                    if(op.O) printf("%d: IN   %4d%4d\n", stats.inst, number, frame_number);
                    stats.ins++;
                }
                else{
                    if(op.O) printf("%d: ZERO     %4d\n", stats.inst, frame_number);
                    stats.zeros++;
                }

            if(op.O) printf("%d: MAP  %4d%4d\n", stats.inst, number, frame_number);
            stats.maps++;
            r->mappage(page_table[number], number, frame_number, frame_table, inverse_table);
            }

            setReferenced(page_table[number],1);
            if(mode == 1) setModified(page_table[number], 1);
        }

        stats.inst++;
    }
}


int main(int argc, char* argv[])
{
    FILE* fh = fopen(argv[argc - 2], "r");
    FILE* rh = fopen(argv[argc - 1], "r");

	if (fh == NULL){
        printf("File does not exist.\n");
        return 0;
	}

    replacement* r = readCommand(argc, argv, rh);
    simulation(fh, r);
    print_all();
    fclose(fh);
    fclose(rh);
    delete r;

    return 0;
}
