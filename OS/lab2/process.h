#ifndef PROCESS_H
#define PROCESS_H


class process
{
    public:
        int pid;
        int arrival_time;
        int total_cpu_time;
        int cpu_burst;
        int io_burst;
        int static_priority;
        int dynamic_priority;
        int finish_time;
        int turnaround_time;
        int io_time;
        int wait_time;
        int remaining_time;
        int remaining_CPU_burst;
        int last_process_time;

        process(int, int, int, int, int, int);  // initializes a process with specifics given in the input file.
        process();
        ~process();
};

#endif // PROCESS_H
