/* Author: Zhe Xu
 * Course: Operating Systems Fall16
 * Assignment: Lab4 IO Scheduling
 * Date: 12/07/2016
 */

#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>

using namespace std;

struct INSTRUCTION{
    int id;
    int r_time;      //request time
    int f_time;      //finish time
    int s_time;     //start time
    int track;
    };

vector<INSTRUCTION> instr_list;     // a list storing all io instructions
vector<INSTRUCTION> iobuffer;       // io queue
vector<INSTRUCTION> backup_buffer;  // backup queue (for fscan only)

void printInstruction(vector<INSTRUCTION>& mylist){
    vector<INSTRUCTION>::iterator it;

    for(it = mylist.begin(); it != mylist.end(); it++)
        printf("%d\t%d\t%d\t%d\t%d\n", it->id, it->r_time, it->s_time, it->f_time, it->track);

    return;
    }

/* Function: compare_id
 * compares the id number of two INSTRUCTIONs.
 * a: the first INSTRUCTION.
 * b: the second INSTRUCTION.
 * returns: true if the id of the first INSTRUCTION is smaller
 * than that of the second INSTRUCTION or false if otherwise.
 */
bool compare_id(INSTRUCTION a, INSTRUCTION b){
    return a.id < b.id;
}

/* Function: compare_ftime
 * compares the finish time of two INSTRUCTIONs.
 * is a helper function for the calculate_stat function.
 * a: the first INSTRUCTION.
 * b: the second INSTRUCTION.
 * returns: true if the finish time of the first INSTRUCTION is earlier
 * than that of the second INSTRUCTION or false if otherwise.
 */
bool compare_ftime(INSTRUCTION a, INSTRUCTION b){
    return a.f_time < b.f_time;
    }

/* Function: compare_track
 * compares the track position of two INSTRUCTIONs.
 * is a helper function for various scheduling function.
 * a: the first INSTRUCTION.
 * b: the second INSTRUCTION.
 * returns: true if the track position of the first INSTRUCTION is larger
 * than that of the second INSTRUCTION or false if otherwise.
 */
bool compare_track(INSTRUCTION a, INSTRUCTION b){
    return a.track > b.track;
    }

/* Function: compare_track_reverse
 * compares the track position of two INSTRUCTIONs.
 * is a helper function for the fscan function.
 * a: the first INSTRUCTION.
 * b: the second INSTRUCTION.
 * returns: true if the track position of the first INSTRUCTION is smaller
 * than that of the second INSTRUCTION or false if otherwise.
 */
bool compare_track_reverse(INSTRUCTION a, INSTRUCTION b){
    return a.track < b.track;
    }

/* Function: calculate_stat
 * calculates and prints out the various statistic information of different scheduling algorithms.
 * flist: the instr_list after all instructinos are finished.
 */
void calculate_stat(vector<INSTRUCTION>& flist){
    vector<INSTRUCTION>::iterator it;
    int total_rtime;
    int total_ftime;
    int total_stime;
    int total_move;
    int current_track;
    int max_wtime = 0;

    sort(flist.begin(), flist.end(), compare_ftime);

    it = flist.begin();
    total_ftime = it->f_time;
    total_rtime = it->r_time;
    total_stime = it->s_time;
    total_move = it->track;
    current_track = it->track;
    it++;

    while(it != flist.end()){
        total_rtime += it->r_time;
        total_ftime += it->f_time;
        total_stime += it->s_time;
        total_move += abs(it->track - current_track);
        if((it->s_time-it->r_time) > max_wtime)
            max_wtime = it->s_time-it->r_time;
        current_track = it->track;
        it++;
        }
    //printf("%d %d %d\n", total_rtime, total_stime, total_ftime);
    double att = (double)(total_ftime-total_rtime)/flist.size();
    double awt = (double)(total_stime-total_rtime)/flist.size();
    printf("SUM: %d %d %.2lf %.2lf %d\n", flist.back().f_time, total_move, att, awt, max_wtime);
    return;
    }

/* Function: readInstruction
 * reads the input file and stores io instructions in the instr_list.
 * file: the input file containing io instructions.
 */
void readInstruction(FILE* file){
    INSTRUCTION temp;
    int id = 0;
    char buffer[300];

    fgets(buffer, 300, file);
    while(!feof(file)){
        if(buffer[0] != '#'){
            sscanf(buffer, "%d%d", &temp.r_time, &temp.track);
            temp.f_time = 0;
            temp.s_time = -1;
            temp.id = id;
            instr_list.push_back(temp);
            id++;
        }
        fgets(buffer, 300, file);
    }
    return;
    }

/* Function: modify
 * finds a given INSTRUCTION in the instr_list and changes its s_time and f_time in the list.
 * is a helper function for scheduling functions.
 * in: the INSTRUCTION needs to be modified.
 */
void modify(INSTRUCTION& in){
    vector<INSTRUCTION>::iterator it;

    for(it = instr_list.begin(); it != instr_list.end(); it++){
        if(it->id == in.id)
            break;
    }

    it->s_time = in.s_time;
    it->f_time = in.f_time;

    return;
}

/* Function: fifo
 * schedules the io instructions with fifo algorithm.
 */
void fifo(){
    vector<INSTRUCTION>::iterator it = instr_list.begin();
    int current_ftime =  it->f_time;
    int current_track = it->track;

    it->f_time = current_ftime = it->r_time + it->track;
    it->s_time = it->r_time;
    it++;
    while(it != instr_list.end()){
        if(it->r_time > current_ftime){
            it->f_time = it->r_time + abs(it->track - current_track);
            it->s_time = it->r_time;
        }
        else{
            it->f_time = current_ftime + abs(it->track - current_track);
            it->s_time = current_ftime;
        }
        current_ftime = it->f_time;
        current_track = it->track;
        it++;
        }
    calculate_stat(instr_list);
    return;
}

/* Function: sstf
 * schedules the io instructions with sstf algorithm.
 */
void sstf(){
    unsigned count = 0;
    int min_dist = INT_MAX;
    bool next_job = false;
    INSTRUCTION first_job;

    vector<INSTRUCTION>::iterator it = instr_list.begin();
    vector<INSTRUCTION>::iterator it2, temp;

    first_job.f_time = it->f_time = it->r_time + it->track;
    first_job.r_time = it->r_time;
    first_job.track = it->track;
    first_job.s_time = it->s_time = it->r_time;

    while(count < instr_list.size()-1){
        for(it = instr_list.begin(); it != instr_list.end(); it++){
            if(it->s_time < 0 && it->r_time <= first_job.f_time){
                next_job = true;
                if(min_dist > abs(it->track-first_job.track)){
                    min_dist = abs(it->track-first_job.track);
                    temp = it;
                    }
            }
        }

        if(next_job){
            temp->s_time = first_job.f_time;
            temp->f_time = first_job.f_time + min_dist;
        }
        else{
            for(it2 = instr_list.begin(); it2 != instr_list.end(); it2++){
                if(it2->s_time < 0)
                    break;
            }
            temp = it2;
            temp->s_time = temp->r_time;
            temp->f_time = temp->r_time + abs(first_job.track-temp->track);
        }

        min_dist = INT_MAX;
        next_job = false;
        first_job = instr_list[temp-instr_list.begin()];
        count++;
    }

    calculate_stat(instr_list);
    return;
}

/* Function: scan
 * schedules the io instructions with scan algorithm.
 */
void scan(){
    unsigned count = 0;
    int direction = 1;
    int last_ftime = 0;
    INSTRUCTION first_job;

    vector<INSTRUCTION>::iterator it1 = instr_list.begin();
    vector<INSTRUCTION>::iterator it2;
    vector<INSTRUCTION>::iterator temp;

    first_job.f_time = it1->f_time = it1->r_time + it1->track;
    first_job.r_time = it1->r_time;
    first_job.track = it1->track;
    first_job.s_time = it1->s_time = it1->r_time;

    while(count < instr_list.size()-1){
        for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
            if(it1->s_time == -1 && it1->r_time > last_ftime && it1->r_time <= first_job.f_time)
                iobuffer.push_back(*it1);
        }

        last_ftime = first_job.f_time;

        if(iobuffer.empty()){
            for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
                if(it1->r_time > first_job.f_time)
                    break;
            }
            iobuffer.push_back(*it1);
        }

        sort(iobuffer.begin(), iobuffer.end(), compare_track);

        if(iobuffer.begin()->track < first_job.track){
            if(direction == 1) direction = -1;
            if(iobuffer.begin()->r_time > first_job.f_time){
                iobuffer.begin()->f_time = iobuffer.begin()->r_time + abs(iobuffer.begin()->track-first_job.track);
                iobuffer.begin()->s_time = iobuffer.begin()->r_time;
            }
            else{
                iobuffer.begin()->f_time = first_job.f_time + abs(iobuffer.begin()->track-first_job.track);
                iobuffer.begin()->s_time = first_job.f_time;
            }
            first_job = *iobuffer.begin();
            modify(first_job);
            iobuffer.erase(iobuffer.begin());
        }
        else{
            for(it2 = iobuffer.begin(); it2 != iobuffer.end(); it2++){
                if(first_job.track >= it2->track)
                    break;
            }
        //find the first instruction whose track is smaller than current track
        if(first_job.track == it2->track && first_job.id != it2->id){          //?????
            it2->f_time = first_job.f_time;
            it2->s_time = first_job.f_time;
            first_job = *it2;
            modify(first_job);
            iobuffer.erase(it2);
        }
        else{
            if(direction == 1){
                it2--;
                if(it2->r_time > first_job.f_time){
                    it2->f_time = it2->r_time + abs(it2->track-first_job.track);
                    it2->s_time = it2->r_time;
                }
                else{
                    it2->f_time = first_job.f_time + abs(it2->track-first_job.track);
                    it2->s_time = first_job.f_time;
                }
                first_job = *it2;
                modify(first_job);
                iobuffer.erase(it2);
            }
            else{
                if(it2 == iobuffer.end()) {
                    direction = 1;
                    it2--;
                    }
                if(it2->r_time > first_job.f_time){
                    it2->f_time = it2->r_time + abs(it2->track-first_job.track);
                    it2->s_time = it2->r_time;
                }
                else{
                    it2->f_time = first_job.f_time + abs(it2->track-first_job.track);
                    it2->s_time = first_job.f_time;
                }
                first_job = *it2;
                modify(first_job);
                iobuffer.erase(it2);
            }
            }
        }
        count++;
    }

    calculate_stat(instr_list);
    return;
}

/* Function: cscan
 * schedules the io instructions with cscan algorithm.
 */
void cscan(){
    unsigned count = 0;
    int last_ftime = 0;
    INSTRUCTION first_job;

    vector<INSTRUCTION>::iterator it1 = instr_list.begin();
    vector<INSTRUCTION>::iterator it2;
    vector<INSTRUCTION>::iterator temp;

    first_job.f_time = it1->f_time = it1->r_time + it1->track;
    first_job.r_time = it1->r_time;
    first_job.track = it1->track;
    first_job.s_time = it1->s_time = it1->r_time;

    while(count < instr_list.size()-1){
        for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
            if(it1->s_time == -1 && it1->r_time > last_ftime && it1->r_time <= first_job.f_time)
                iobuffer.push_back(*it1);
        }

        last_ftime = first_job.f_time;

        if(iobuffer.empty()){
            for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
                if(it1->r_time > first_job.f_time)
                    break;
            }
            iobuffer.push_back(*it1);
        }

        sort(iobuffer.begin(), iobuffer.end(), compare_track);

        if(iobuffer.begin()->track < first_job.track){
            if(iobuffer.back().r_time > first_job.f_time){
                iobuffer.back().f_time = iobuffer.back().r_time + abs(iobuffer.back().track-first_job.track);
                iobuffer.back().s_time = iobuffer.back().r_time;
            }
            else{
                iobuffer.back().f_time = first_job.f_time + abs(iobuffer.back().track-first_job.track);
                iobuffer.back().s_time = first_job.f_time;
            }
            first_job = iobuffer.back();
            modify(first_job);
            iobuffer.erase(iobuffer.end()-1);
        }
        else{
            for(it2 = iobuffer.begin(); it2 != iobuffer.end(); it2++){
                if(first_job.track > it2->track)             //?????
                    break;
            }
        //find the first instruction whose track is smaller than current track

            it2--;
            if(it2->r_time > first_job.f_time){
                it2->f_time = it2->r_time + abs(it2->track-first_job.track);
                it2->s_time = it2->r_time;
            }
            else{
                it2->f_time = first_job.f_time + abs(it2->track-first_job.track);
                it2->s_time = first_job.f_time;
            }
            first_job = *it2;
            modify(first_job);
            iobuffer.erase(it2);

        }
        count++;
    }

    calculate_stat(instr_list);
    return;
}

/* Function: fscan
 * schedules the io instructions with fscan algorithm.
 */
void fscan(){
    unsigned count = 0;
    int last_ftime = 0;
    INSTRUCTION first_job;

    vector<INSTRUCTION>::iterator it1 = instr_list.begin();
    vector<INSTRUCTION>::iterator it2;
    vector<INSTRUCTION>::iterator temp;

    first_job.f_time = it1->f_time = it1->r_time + it1->track;
    first_job.r_time = it1->r_time;
    first_job.track = it1->track;
    first_job.s_time = it1->s_time = it1->r_time;

    for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
        if(it1->s_time == -1 && it1->r_time > last_ftime && it1->r_time <= first_job.f_time)
            iobuffer.push_back(*it1);
        }

    while(count < instr_list.size()-1){
        //printInstruction(iobuffer);
        sort(iobuffer.begin(), iobuffer.end(), compare_track_reverse);
        count += iobuffer.size();
        while(!iobuffer.empty()){
            bool up = false;
            last_ftime = first_job.f_time;
            for(it2 = iobuffer.begin(); it2 != iobuffer.end(); it2++){
                if(it2->track >= first_job.track){
                    up = true;
                    it2->s_time = last_ftime;
                    it2->f_time = last_ftime + abs(first_job.track - it2->track);
                    break;
                }
                }
            if(up){
                first_job = *it2;
                modify(first_job);
                iobuffer.erase(it2);
            }
            else{
                iobuffer.back().s_time = last_ftime;
                iobuffer.back().f_time = last_ftime + abs(first_job.track - iobuffer.back().track);
                first_job = iobuffer.back();
                modify(first_job);
                iobuffer.erase(iobuffer.end()-1);
            }
        }
        //printf("switch to backup: %d\n", first_job.f_time);
        for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
            if(it1->s_time == -1 && it1->r_time <= first_job.f_time)
                backup_buffer.push_back(*it1);
        }
        //printInstruction(backup_buffer);
        sort(backup_buffer.begin(), backup_buffer.end(), compare_track_reverse);
        count += backup_buffer.size();
        while(!backup_buffer.empty()){
            bool up = false;
            last_ftime = first_job.f_time;
            for(it2 = backup_buffer.begin(); it2 != backup_buffer.end(); it2++){
                if(it2->track >= first_job.track){
                    up = true;
                    it2->s_time = last_ftime;
                    it2->f_time = last_ftime + abs(first_job.track - it2->track);
                    break;
                }
            }
                if(up){
                    first_job = *it2;
                    modify(first_job);
                    backup_buffer.erase(it2);
                }
                else{
                    backup_buffer.back().s_time = last_ftime;
                    backup_buffer.back().f_time = last_ftime + abs(first_job.track - backup_buffer.back().track);
                    first_job = backup_buffer.back();
                    modify(first_job);
                    backup_buffer.erase(backup_buffer.end()-1);
                }
        }
        //printf("switch to first queue: %d\n", first_job.f_time);
        for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
            if(it1->s_time == -1 && it1->r_time <= first_job.f_time)
                iobuffer.push_back(*it1);
        }
        //printInstruction(iobuffer);
        if(iobuffer.empty() && backup_buffer.empty()){
            bool next = false;
            for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
                if(it1->s_time == -1 && it1->r_time > first_job.f_time){
                    next = true;
                    break;
                }
            }
            if(next){
                it1->s_time = it1->r_time;
                it1->f_time = it1->r_time + abs(first_job.track - it1->track);
                first_job = *it1;
                modify(first_job);
                last_ftime = first_job.f_time;
                count++;
                for(it1 = instr_list.begin(); it1 != instr_list.end(); it1++){
                    if(it1->s_time == -1 && it1->r_time <= first_job.f_time)
                        iobuffer.push_back(*it1);
                }
            }
        }
    }
    calculate_stat(instr_list);
    return;
}

int main(int argc, char* argv[]){
    if(argc < 2){
        puts("missing argument");
        return 0;
        }

    FILE* fh;
    fh = fopen(argv[argc-1], "r");

	if (fh == NULL){
        printf("file does not exists");
        return 0;
	}

    readInstruction(fh);

    int c;
    while((c = getopt(argc, argv, "s:"))!= -1){
        if(c == 's'){
            switch(optarg[0]){
                case 'i':
                    fifo();
                    break;
                case 'j':
                    sstf();
                    break;
                case 's':
                    scan();
                    break;
                case 'c':
                    cscan();
                    break;
                case 'f':
                    fscan();
                    break;
                default:
                    puts("invalid argument");
            }
        }
        else puts("invalid argument");
    }
    fclose(fh);
    return 0;
}
