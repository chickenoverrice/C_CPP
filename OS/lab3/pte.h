#ifndef PTE_H_INCLUDED
#define PTE_H_INCLUDED

#include <vector>

// page table entry
struct PTE {
    unsigned int address;
};

// functions to access or modify corresponding bits in PTE
int getPresent(PTE);
int getModified(PTE);
int getReferenced(PTE);
int getPageout(PTE);
int getRead(PTE);
int getWrite(PTE);
unsigned int getPFN(PTE);
int getCaching(PTE);
void setPresent(PTE&, int);
void setModified(PTE&, int);
void setReferenced(PTE&, int);
void setPageout(PTE&, int);
void setPFN(PTE&, unsigned int);
void setRead(PTE&, int);
void setWrite(PTE&, int);
void setCaching(PTE&, int);
#endif // PTE_H_INCLUDED

