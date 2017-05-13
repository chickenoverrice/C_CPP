#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <algorithm>
#include <climits>
#include "process.h"
#include "scheduler.h"

#define BUFSIZE 100
using namespace std;

// process states
enum TRANSITION {created_ready, ready_running, running_blocked, running_ready, blocked_ready, finished};

// discrete events
struct EVENT{
        int timestamp;
        int pid;
        int start_time;
        TRANSITION state;
};

// io block
struct BLOCK{
        int start;
        int stop;
};

vector<EVENT> DESQueue;     // event queue
vector<process> PQueue;     // process queue
vector<BLOCK> block_info;   // io block queue for calculating io utilization

int lineNumber = 0;         // used for getRandomNumber

/* Function: getRandomNumber
 * reads a number from the given rfile of the Assignment.
 * random_file: a file pointer to the file containing random numbers.
 * returns a random number.
 */
int getRandomNumber(FILE* random_file){
    char *buffer = NULL;
    size_t len;
    int number;

    if(lineNumber == 0) getline(&buffer, &len, random_file);
        getline(&buffer, &len, random_file);

    if(feof(random_file)){
        fseek(random_file, 0, SEEK_SET);
        getline(&buffer, &len, random_file);
        getline(&buffer, &len, random_file);
        }

    sscanf(buffer, "%d", &number);
    //printf("%d\n", number);
    return number;
}

/* Function: compare_io
 * compares the start time of two io events,
 * to be used for sorting io events.
 * a and b are two BLOCK structs for comparison.
 * returns true if a starts earlier than b.
 */
bool compare_io(BLOCK a, BLOCK b){
    return a.start < b.start;
}


/* Function: ioUtilization
 * calculates the percentage of time when io is occupied.
 * returns the io utilization number.
 */
double ioUtilization(int finishTime){
    int max_end = 0;
    int idle = 0;
    vector<BLOCK>::iterator it;

    sort(block_info.begin(), block_info.end(), compare_io);

    if(block_info.begin()->start > 0) idle = idle + block_info.begin()->start;

    for(it = block_info.begin(); it != (block_info.end() -1); it++){
        if(it->stop > max_end)
            max_end = it->stop;
        if((it+1)->start > max_end)
            idle += (it+1)->start - max_end;
    }

    if(block_info.back().stop > max_end)
        max_end = block_info.back().stop;

    //if(finishTime > max_end) idle = idle + finishTime - max_end;

    return (double)(max_end - idle)*100/(double)finishTime;
}


/* Function: addEvent
 * adds an EVENT to the event queue DESQueue in chronological order.
 * e: an arriving event.
 */
void addEvent(EVENT e){
    vector<EVENT>::iterator it;
    for(it = DESQueue.begin(); it != DESQueue.end(); it++){
        if(it->timestamp > e.timestamp)
            break;
    }
    DESQueue.insert(it, e);
    return;
}


/* Function getEvent
 * extracts an event from the DESQueue.
 * returns an EVENT.
 */
EVENT getEvent(){
    EVENT e ;
    if(!DESQueue.empty()){
        e = DESQueue.front();
        DESQueue.erase(DESQueue.begin());
    }
    return e;
}

/* Function createProcess
 * reads a process from input file and adds the process to the process queue.
 * input_file: a file pointer to the file containing process information.
 * random_file: a file pointer to the file containing random numbers.
 */
void createProcess(FILE* input_file, FILE* random_file){
    char buffer[BUFSIZE];
    int id = 0;

    while (fgets(buffer, BUFSIZE, input_file) != NULL) {
        int art, tct, cb, ib, sp;
        sp = 1 + getRandomNumber(random_file) % 4;
        lineNumber++;
        sscanf(buffer, "%d%d%d%d", &art, &tct, &cb, &ib);
        PQueue.push_back(process(id++, art, tct, cb, ib, sp));
    }
}

/* Function createDESQueue
 * creates event queue.
 */
void createDESQueue(){
    vector<process>::iterator it;
    EVENT temp;

    for(it = PQueue.begin(); it != PQueue.end(); it++){
        temp.pid = it->pid;
        temp.timestamp = it->arrival_time;
        temp.start_time = 0;
        temp.state= created_ready;
        addEvent(temp);
    }
}

/* Function runProcessFinish
 * changes the status of a process to finished.
 * is a helper function for the function simulation.
 * time: a time point.
 * i: index to the process queue PQueue.
 * nextjob: the time point for the next scheduled job.
 */
void runProcessFinish(int time, int i, int& nextjob){
    nextjob = time + PQueue[i].remaining_time;
    EVENT e = {nextjob, i, time, finished};
    addEvent(e);
    PQueue[i].remaining_time = 0;
}

/* Function runProcessBlock
 * changes the status of a process to blocked.
 * is a helper function for the function simulation.
 * time: a time point.
 * cb: cpu burst.
 * i: index to the process queue PQueue.
 * nextjob: the time point for the next scheduled job.
 */
void runProcessBlock(int time, int cb, int i, int& nextjob){
    nextjob = time + cb;
    EVENT e ={nextjob, i, time, running_blocked};
    addEvent(e);
    PQueue[i].remaining_time = PQueue[i].remaining_time - cb;
}

/* Function runProcessReady
 * changes the status of a process to ready.
 * is a helper function for the function simulation.
 * only affects RR and PRIO scheduler.
 * time: a time point.
 * cb: cpu burst.
 * quantum: quantum assigned by the user.
 * i: index to the process queue PQueue.
 * nextjob: the time point for the next scheduled job.
 */
void runProcessReady(int time, int cb, int quantum, int i, int& nextjob){
    EVENT e = {time + quantum, i, time, running_ready};
    if(quantum == cb)
        e.state = running_blocked;

    addEvent(e);
    nextjob = time + quantum;
    PQueue[i].remaining_CPU_burst = cb - quantum;
    PQueue[i].remaining_time = PQueue[i].remaining_time - quantum;
}

/* Function runQuantumScheduler
 * changes the status of a process to running.
 * is a helper function for the function simulation.
 * only affects RR and PRIO scheduler.
 * time: a time point.
 * quantum: quantum assigned by the user.
 * i: index to the process queue PQueue.
 * nextjob: the time point for the next scheduled job.
 * random_file: a file pointer to the file containing random numbers.
 */
void runQuantumScheduler(int time, int quantum, int i, int& nextjob, FILE* random_file){
    int cb = 0;
    int cpuburst;
    if(PQueue[i].remaining_CPU_burst == 0){
        cb = 1 + getRandomNumber(random_file) % PQueue[i].cpu_burst;
        lineNumber++;
        PQueue[i].remaining_CPU_burst = cb;
        }
    cpuburst = PQueue[i].remaining_CPU_burst;

    if(quantum <= cpuburst){
        if(PQueue[i].remaining_time <= quantum)
            runProcessFinish(time, i, nextjob);
        else
            runProcessReady(time, cpuburst, quantum, i, nextjob);
    }
    else{
        if(PQueue[i].remaining_time <= cpuburst)
            runProcessFinish(time, i, nextjob);
        else
            runProcessBlock(time, cpuburst, i, nextjob);
        PQueue[i].remaining_CPU_burst = 0;
    }
}


/* Function simulation
 * tests the result of process scheduling with different scheduling algorithms.
 * sched: a pointer to a scheduler object.
 * random_file: a file pointer to the file containing random numbers.
 */
void simulation(scheduler* sched, FILE* random_file){
    EVENT current_job = {0,0,0,created_ready};
    int current_time = PQueue.front().arrival_time;
    int next_job_time = 0;
    int ioburst = 0;
    int cpuburst = 0;
    //puts("OK");

    while(!DESQueue.empty()){
        current_job = getEvent();
        int i = current_job.pid;

        if(current_job.timestamp > current_time)
            current_time = current_job.timestamp;

        if(current_job.state == created_ready){
            EVENT e = {PQueue[i].arrival_time, i, PQueue[i].arrival_time, ready_running};
            addEvent(e);
            PQueue[i].last_process_time = current_time;
            sched->add_process(PQueue[i]);
        }

        else if(current_job.state == ready_running){
            if(current_job.timestamp < next_job_time){
                current_job.timestamp = next_job_time;
                addEvent(current_job);
                continue;
            }

            process p = sched->get_next_process();
            i = p.pid;
            current_job.start_time = PQueue[i].last_process_time;
            PQueue[i].wait_time = PQueue[i].wait_time + current_time - current_job.start_time;

            if(sched->quantum < INT_MAX)
                runQuantumScheduler(current_time, sched->quantum, i, next_job_time, random_file);
            else {
                cpuburst = 1 + getRandomNumber(random_file) % PQueue[i].cpu_burst;
                lineNumber++;
                //printf("CPUBurst: %d\n", cpuburst);
                PQueue[i].remaining_CPU_burst = cpuburst;

                if(PQueue[i].remaining_time <= cpuburst)
                    runProcessFinish(current_time, i, next_job_time);
                else
                    runProcessBlock(current_time, cpuburst, i, next_job_time);
            }
        }

        else if((current_job.state == blocked_ready) || (current_job.state == running_ready)){
            int dp = PQueue[i].dynamic_priority;
            int sp = PQueue[i].static_priority;
            EVENT e = {current_time, i, current_time, ready_running};
            addEvent(e);
            PQueue[i].last_process_time = current_time;
            if(current_job.state == running_ready)
                PQueue[i].dynamic_priority = dp - 1;
            else PQueue[i].dynamic_priority = sp - 1;

            sched->add_process(PQueue[i]);
        }

        else if(current_job.state == finished){
            PQueue[i].finish_time = current_time;
        }

        else if(current_job.state == running_blocked){
            BLOCK b;
            ioburst = 1 + getRandomNumber(random_file) % PQueue[i].io_burst;
            lineNumber++;
            //printf("IOBurst: %d\n", ioburst);
            EVENT e = {current_time + ioburst, i, current_time, blocked_ready};
            addEvent(e);
            PQueue[i].io_time = PQueue[i].io_time + ioburst;

            b.start = current_time;
            b.stop = current_time + ioburst;
            block_info.push_back(b);
        }

    }
    return;
}

/* Function printStatistics
 * calculates and prints out the statistics of different scheduling algorithms.
 * sched: a pointer to a scheduler object used in the simulation part.
 */
void printStatistics(scheduler* sched){
    int last = -1;
    int turnaround_time = 0;
    int cpuwaiting_time = 0;
    int cpubusy_time = 0;
    double cpuUtil = 0.0;
    double ave_turnaround;
    double ave_wait;
    double throughput;

    vector<process>::iterator it;

    printf("%s", sched->get_name());
    if(sched->name == 'R' || sched->name == 'P')
        printf(" %d", sched->quantum);
    printf("\n");

    for(it = PQueue.begin(); it != PQueue.end(); it++){
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
                it->pid, it->arrival_time, it->total_cpu_time, it->cpu_burst,
                it->io_burst, it->static_priority, it->finish_time, it->finish_time - it->arrival_time,
                it->io_time, it->wait_time);
        if (it->finish_time > last)
            last = it->finish_time;

        turnaround_time += it->finish_time - it->arrival_time;
        cpuwaiting_time += it->wait_time;
        cpubusy_time += it->total_cpu_time;
    }

    cpuUtil = (double)cpubusy_time * 100 / (double) last;
    ave_turnaround = (double)turnaround_time / (double) PQueue.size();
    ave_wait = (double)cpuwaiting_time / (double)PQueue.size();
    throughput = (double)PQueue.size()*100/(double)last;
    double ioUtil = ioUtilization(last);

    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", last, cpuUtil, ioUtil, ave_turnaround, ave_wait, throughput);

    return;
}

/* Function readCommand
 * reads the user specified options for scheduler type and quantum.
 * returns the corresponding scheduler.
 */
scheduler* readCommand(int argc, char* argv[]){
    int c;
    int quantum;
    char temp[10];
    while((c = getopt(argc, argv, "s:"))!= -1){
        if(c == 's'){
            switch(optarg[0]){
                case 'F':
                    //puts("FCFS");
                    return new FCFSscheduler('F');
                    break;
                case 'L':
                    return new LCFSscheduler('L');
                    break;
                case 'S':
                    return new SJFscheduler('S');
                    break;
                case 'R':
                    strncpy(temp, optarg+1, sizeof(optarg)-1);
                    sscanf(temp, "%d", &quantum);
                    return new RRscheduler('R', quantum);
                    break;
                case 'P':
                    strncpy(temp, optarg+1, sizeof(optarg)-1);
                    sscanf(temp, "%d", &quantum);
                    return new PRIOscheduler('P', quantum);
                    break;
                default:
                    break;
            }
        }
        else {
            puts("invalid argument");
            return NULL;
            }
    }
    return NULL;
}



int main(int argc, char* argv[])
{
    if(argc < 3){
        puts("missing argument");
        return 0;
        }

    FILE* input_file;
    FILE* random_file;

    input_file = fopen(argv[argc-2], "r");
    random_file = fopen(argv[argc-1], "r");

	if (input_file == NULL){
        printf("input file does not exist");
        return 0;
	}

	if (random_file == NULL){
        printf("random file does not exist");
        return 0;
	}

    createProcess(input_file, random_file);
    createDESQueue();
    scheduler* sched = readCommand(argc, argv);
    simulation(sched, random_file);
    printStatistics(sched);
    fclose(input_file);
    fclose(random_file);
    delete(sched);
    return 0;
}
