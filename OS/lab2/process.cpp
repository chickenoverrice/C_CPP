#include "process.h"

// initializes a process with given specifics from input file.
process::process(int id, int art, int tct, int cb, int ib, int sp){
    pid = id;
    arrival_time = art;
    total_cpu_time = tct;
    cpu_burst = cb;
    io_burst = ib;
    remaining_time = tct;
    static_priority = sp;
    dynamic_priority = sp - 1;
    finish_time = 0;
    turnaround_time = 0;
    io_time = 0;
    wait_time = 0;
    remaining_CPU_burst = 0;
    last_process_time = 0;
}

// initializes a temporal process.
process::process(){
    pid = 0;
    arrival_time = 0;
    total_cpu_time = 0;
    cpu_burst = 0;
    io_burst = 0;
    static_priority = 0;
    dynamic_priority = 0;
    finish_time = 0;
    turnaround_time = 0;
    io_time = 0;
    wait_time = 0;
    remaining_time = 0;
    remaining_CPU_burst = 0;
    last_process_time = 0;
}

process::~process()
{
    //dtor
}
