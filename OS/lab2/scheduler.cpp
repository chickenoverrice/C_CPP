#include "scheduler.h"
#include <stdio.h>
#include <climits>

scheduler::scheduler(char c){
    name = c;
    quantum = INT_MAX;
}

scheduler::~scheduler()
{
    //dtor
}

void scheduler::setQueue(std::vector<process> input_queue){
    active_queue = input_queue;
}

std::vector<process> scheduler::getQueue(){
    return active_queue;
}

FCFSscheduler::FCFSscheduler(char c):scheduler(c){}
LCFSscheduler::LCFSscheduler(char c):scheduler(c){}
SJFscheduler::SJFscheduler(char c):scheduler(c){}
RRscheduler::RRscheduler(char c, int q):scheduler(c){quantum = q;}
PRIOscheduler::PRIOscheduler(char c, int q):scheduler(c){quantum = q;}
FCFSscheduler::~FCFSscheduler(){}
LCFSscheduler::~LCFSscheduler(){}
SJFscheduler::~SJFscheduler(){}
RRscheduler::~RRscheduler(){}
PRIOscheduler::~PRIOscheduler(){}
const char* FCFSscheduler::get_name(){return "FCFS";}
const char* LCFSscheduler::get_name(){return "LCFS";}
const char* SJFscheduler::get_name(){return "SJF";}
const char* RRscheduler::get_name(){return "RR";}
const char* PRIOscheduler::get_name(){return "PRIO";}

process FCFSscheduler::get_next_process(){
    process p;
    if(!active_queue.empty()){
        p = active_queue.front();
        active_queue.erase(active_queue.begin());
    }
    return p;
}

process LCFSscheduler::get_next_process(){
    process p;
    if(!active_queue.empty()){
        p = active_queue.back();
        active_queue.pop_back();
    }
    return p;
}

process SJFscheduler::get_next_process(){
    process p;
    std::vector<process>::iterator it, temp;
    int shortest = INT_MAX;
    if(!active_queue.empty()){
        for(it = active_queue.begin(); it != active_queue.end(); it++){
            if(it->remaining_time < shortest){
                temp = it;
                shortest = it->remaining_time;
            }
        }
    }
    p = *temp;
    active_queue.erase(temp);
    return p;
}

process RRscheduler::get_next_process(){
    process p;
    if(!active_queue.empty()){
        p = active_queue.front();
        active_queue.erase(active_queue.begin());
    }
    return p;
}

void PRIOscheduler::add_process(process& p){
    if(p.dynamic_priority >= 0)
        active_queue.push_back(p);
    else{
        p.dynamic_priority = p.static_priority-1;
        expire_queue.push_back(p);
        }
    return;
}

process PRIOscheduler::get_next_process(){
    process p;
    std::vector<process>::iterator it, temp;
    int highest = -1;
    if(active_queue.empty()){
        active_queue = expire_queue;
        expire_queue.clear();
    }

    if(!active_queue.empty()){
        for(it = active_queue.begin(); it != active_queue.end(); it++){
            if(it->dynamic_priority > highest){
                temp = it;
                highest = it->dynamic_priority;
            }
        }
    }
    p = *temp;
    active_queue.erase(temp);
    return p;
}


void RRscheduler::add_process(process& p){
    active_queue.push_back(p);
    return;
}

void FCFSscheduler::add_process(process& p){
    active_queue.push_back(p);
    return;
}

void LCFSscheduler::add_process(process& p){
    active_queue.push_back(p);
    return;
}

void SJFscheduler::add_process(process& p){
    active_queue.push_back(p);
    return;
}

