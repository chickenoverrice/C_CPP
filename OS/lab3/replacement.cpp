#include "replacement.h"
#include <stdio.h>

replacement::replacement(){
    firstLine = true;
}

replacement::~replacement(){}

void replacement::usage(vector<unsigned int>& ftable, unsigned int fnumber){}


int replacement::getRandomNumber(){
    char *buffer = NULL;
    size_t len;
    int number;

    if(firstLine) getline(&buffer, &len, random_file);

    if(getline(&buffer, &len, random_file) < 0){
        fseek(random_file, 0, SEEK_SET);
        getline(&buffer, &len, random_file);
        getline(&buffer, &len, random_file);
        }

    sscanf(buffer, "%d", &number);
    //printf("%d\n", number);
    return number;

}

void replacement::unmappage(PTE& addr, vector<unsigned int>& ftable, vector<unsigned int>& itable){
    setPresent(addr, 0);
    setReferenced(addr, 0);
}

void replacement::mappage(PTE& addr, unsigned int vpn, unsigned int change, vector<unsigned int>& ftable, vector<unsigned int>& itable){
    setPFN(addr, change);
    setPresent(addr, 1);
    itable[change] = vpn;
}

unsigned int FIFO::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int vpn = ftable.front();
    ftable.erase(ftable.begin());
    ftable.push_back(vpn);
    return ftable.back();
}

unsigned int Second::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int vpn = ftable.front();
    unsigned int index = itable[vpn];
    while (getReferenced(ptable[index]) != 0) {
        setReferenced(ptable[index],0);
        ftable.erase(ftable.begin());
        ftable.push_back(vpn);
        vpn = ftable.front();
        index = itable[vpn];
    }
    ftable.erase(ftable.begin());
    ftable.push_back(vpn);
    return ftable.back();
}

NRU::NRU(FILE* r_file){
    firstLine = true;
    random_file = r_file;
    timer = 0;
}

unsigned int NRU::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    vector<PTE> q0, q1, q2, q3;
    unsigned int vpn;
    PTE page;
    int number;
    number = getRandomNumber();

    for(int i = 0; i < 63; i++){
        if(getPresent(ptable[i]) == 1){
            unsigned int r = getReferenced(ptable[i]);
            unsigned int m = getModified(ptable[i]);
            if(m == 0 && r == 0) q0.push_back(ptable[i]);
            else if(m == 1 && r == 0) q1.push_back(ptable[i]);
            else if(m == 0 && r == 1) q2.push_back(ptable[i]);
            else q3.push_back(ptable[i]);
        }
    }

    if(!q0.empty()) page = q0[number % q0.size()];
    else if(!q1.empty()) page = q1[number % q1.size()];
    else if(!q2.empty()) page = q2[number % q2.size()];
    else page = q3[number % q3.size()];
    vpn = getPFN(page);

    timer++;
    if(timer == 10){
        for(int i = 0; i < 63; i++){
            if(getPresent(ptable[i]) == 1)
                setReferenced(ptable[i], 0);
        }
        timer = 0;
    }
    firstLine = false;
    return vpn;
}

Random::Random(FILE* r_file){firstLine = true; random_file = r_file;}

unsigned int Random::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int vpn = ftable[getRandomNumber() % ftable.size()];
    firstLine = false;
    return vpn;
}

ClockP::ClockP(){timer = 0;}
ClockV::ClockV(){timer = 0;}

unsigned int ClockV::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int vpn;
    bool found = false;
    int index;

    while(!found){
        index = timer % 64;
        if(getReferenced(ptable[index]) == 0 && getPresent(ptable[index]) == 1){
            vpn = getPFN(ptable[index]);
            found = true;
        }
        if(getReferenced(ptable[index]) == 1 && getPresent(ptable[index]) == 1)
            setReferenced(ptable[index], 0);
        timer++;
    }
    return vpn;
}

unsigned int ClockP::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int vpn;
    bool found = false;
    int index, page_index;

    while(!found){
        index = timer % ftable.size();
        page_index = itable[ftable[index]];
        if(getReferenced(ptable[page_index]) == 0){
            vpn = ftable[index];
            found = true;
        }
        else
            setReferenced(ptable[page_index], 0);
        timer++;
    }
    return vpn;
}

AgingP::AgingP(){
    for(int i = 0; i < 64; i++)
        counter[i] = 0;
}

AgingV::AgingV(){
    for(int i = 0; i < 64; i++)
        counter[i] = 0;
}

unsigned int AgingP::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int youngest = 0xffffffff;
    int index = -1;
    int page_index;

    for(unsigned int i = 0; i < ftable.size(); i++){
        page_index = itable[ftable[i]];
        counter[i] = (counter[i] >> 1) | (getReferenced(ptable[page_index]) << 31);
        if(counter[i] <youngest){
            youngest = counter[i];
            index = i;
        }
        setReferenced(ptable[page_index], 0);
    }

    counter[index] = 0;
    return ftable[index];
}

unsigned int AgingV::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int vpn;
    unsigned int youngest = 0xffffffff;
    int index = -1;

    for(int i = 0; i < 64; i++){
        counter[i] = (counter[i] >> 1) | (getReferenced(ptable[i]) << 31);
        if((getPresent(ptable[i]) == 1) && (counter[i] < youngest)){
            youngest = counter[i];
            index = i;
        }
        if(getPresent(ptable[i]) == 1)
            setReferenced(ptable[i], 0);
    }

    counter[index] = 0;
    vpn = getPFN(ptable[index]);
    return vpn;
}

LRU::LRU(){}

void LRU::usage(vector<unsigned int>& ftable, unsigned int fnumber){
    for(unsigned int i = 0; i < ftable.size(); i++){
        if(fnumber == ftable[i]){
            ftable.erase(ftable.begin() + i);
            break;
        }
    }
    ftable.push_back(fnumber);
}


unsigned int LRU::selectPage(PTE ptable[], vector<unsigned int>& ftable, vector<unsigned int>& itable){
    unsigned int vpn = ftable.front();
    ftable.erase(ftable.begin());
    ftable.push_back(vpn);
    return vpn;
}

