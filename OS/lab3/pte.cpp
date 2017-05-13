#include "pte.h"

int getPresent(PTE addr){
    return (addr.address & ( 1 << 6)) >> 6;
};

int getModified(PTE addr){
    return (addr.address & ( 1 << 9)) >> 9;
};

int getReferenced(PTE addr){
    return (addr.address & ( 1 << 10)) >> 10;
};

/* what is pageout bit?? */
int getPageout(PTE addr){
    return (addr.address & ( 1 << 11)) >> 11;
};

int getRead(PTE addr){
    return (addr.address & ( 1 << 8)) >> 8;
};

int getWrite(PTE addr){
    return (addr.address & ( 1 << 7)) >> 7;
};

unsigned int getPFN(PTE addr){
    return addr.address & 0x3F;
};

int getCaching(PTE addr){
    return (addr.address & ( 1 << 12)) >> 12;
};

void setPresent(PTE& addr, int change){
    addr.address ^= (-change ^ addr.address) & (1 << 6);
};

void setModified(PTE& addr, int change){
    addr.address ^= (-change ^ addr.address) & (1 << 9);
};

void setReferenced(PTE& addr, int change){
    addr.address ^= (-change ^ addr.address) & (1 << 10);
};

void setPageout(PTE& addr, int change){
    addr.address ^= (-change ^ addr.address) & (1 << 11);
};

void setPFN(PTE& addr, unsigned int change){
    addr.address = (addr.address & 0xFFFFFFC0) + change;
};

void setRead(PTE& addr, int change){
    addr.address ^= (-change ^ addr.address) & (1 << 8);
};

void setWrite(PTE& addr, int change){
    addr.address ^= (-change ^ addr.address) & (1 << 7);
};

void setCaching(PTE& addr, int change){
    addr.address ^= (-change ^ addr.address) & (1 << 12);
};

