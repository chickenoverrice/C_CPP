#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "process.h"

class scheduler
{
    public:
        scheduler(char);
        virtual ~scheduler();

        void setQueue(std::vector<process>);      // initializes the ready queue.
        std::vector<process> getQueue();          // returns the ready queue.
        virtual const char* get_name() = 0;       // returns the full name of the scheduler.
        virtual process get_next_process() = 0;   // returns the process choosen by the scheduler.
        virtual void add_process(process&) = 0;   // adds a process to the ready queue.

        char name;
        std::vector<process> active_queue;
        int quantum;
};

class FCFSscheduler:public scheduler {
    public:
        ~FCFSscheduler();
        FCFSscheduler(char);
        const char* get_name();
        void add_process(process&);
        process get_next_process();
};

class LCFSscheduler:public scheduler {
    public:
        LCFSscheduler(char);
        ~LCFSscheduler();
        const char* get_name();
        void add_process(process&);
        process get_next_process();
};

class SJFscheduler:public scheduler {
    public:
        SJFscheduler(char);
        ~SJFscheduler();
        const char* get_name();
        void add_process(process&);
        process get_next_process();
};

class RRscheduler:public scheduler {
    public:
        RRscheduler(char, int);
        ~RRscheduler();
        const char* get_name();
        void add_process(process&);
        process get_next_process();
};

class PRIOscheduler:public scheduler {
    public:
        PRIOscheduler(char, int);
        ~PRIOscheduler();
        const char* get_name();
        void add_process(process&);
        process get_next_process();

    private:
        std::vector<process> expire_queue;
};

#endif // SCHEDULER_H
