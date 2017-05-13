#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include "pte.h"
#include <stdio.h>
#include <vector>

using namespace std;

// base class
class replacement
{
    public:
        replacement();
        virtual ~replacement();

        // un-maps a page
        void unmappage(PTE&, vector<unsigned int>&, vector<unsigned int>&);
        // maps a page to a given frame
        void mappage(PTE&, unsigned int, unsigned int, vector<unsigned int>&, vector<unsigned int>&);
        // selects a page to be swapped out. algorithm depedent.
        virtual unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&) = 0;
        // records the usage of pages. only applied to LRU.
        virtual void usage(vector<unsigned int>&, unsigned int);
        // reads a random number. only used by Random and NRU.
        int getRandomNumber();

        FILE* random_file;              // file containing random numbers
        unsigned int frame_size;        // user defined frame size
        bool firstLine;
};

class NRU: public replacement{
    public:
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
        NRU(FILE*);
    private:
        int timer;
};

class LRU: public replacement{
    public:
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
        LRU();
    private:
        void usage(vector<unsigned int>&, unsigned int);
};

class FIFO: public replacement{
    public:
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
};

class ClockP: public replacement{
    public:
        ClockP();
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
    private:
        int timer;
};

class ClockV: public replacement{
    public:
        ClockV();
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
    private:
        int timer;
};

class Random: public replacement{
    public:
        Random(FILE*);
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
};

class AgingP: public replacement{
    public:
        AgingP();
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
    private:
        unsigned int counter[64];
};

class Second: public replacement{
    public:
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
};

class AgingV: public replacement{
    public:
        AgingV();
        unsigned int selectPage(PTE [], vector<unsigned int>&, vector<unsigned int>&);
    private:
        unsigned int counter[64];
};
#endif // REPLACEMENT_H

