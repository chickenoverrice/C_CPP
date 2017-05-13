#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>

using namespace std;

struct BLOCK {
    unsigned int id;            //Basic block ID
    unsigned long long stop;    //The last instruction in a block
    unsigned int num_instr;     //The number of instructions inside a block
    unsigned int num_use;       //The number of times a block is executed
};

struct FLOW {
    unsigned int from_id;       //The starting block ID of a flow
    unsigned int to_id;         //The ending block ID of a flow
};

struct comp {
    bool operator() (const FLOW& f1, const FLOW& f2) const{
        if(f1.from_id<f2.from_id) return true;
        if(f1.from_id>f2.from_id) return false;
        return f1.to_id<f2.to_id;}
};

map<unsigned long long, BLOCK> block_info;  //Map a leader instruction to a basic block
map<unsigned long long, int> branch;        //Record the addresses of all branch instructions
map<unsigned long long, int> leader;        //Record the addresses of all leader instructions
map<FLOW, unsigned int, comp> flow_info;    //Map a starting block to an ending block
vector<unsigned long long> instruction;     //Store all 'I' type instructions from trace files
vector<bool> isleader;                      //Mark whether an instruction is a leader or not


//Get an entire instruction and extract the address and size.
void parseInstruction(string s, unsigned long long* address, unsigned int* length){
    int counter=0;
    char* pEnd;
    char * dup = strdup(s.c_str());
    char * token = strtok(dup, " ,");
    while(token != NULL){
        if(counter==1){
            (*address)=strtoull(token,&pEnd,16);  //Get the address of an instruction.
        }
        else if(counter==2){
            (*length)=(unsigned int)strtoul(token,&pEnd,10);  //Get the size of an instruction.
        }
        token = strtok(NULL, " ,");
        counter++;
    }
    free(dup);
}

//Identify leader instructions.
void mark_leader(){
    map<unsigned long long, int>::iterator it1, it2;
    leader[instruction[0]]=1;  //The first instruction is a leader.
    isleader[0]=true;

    for(unsigned int i=1; i<isleader.size(); i++){
        it1=leader.find(instruction[i]);
        it2=branch.find(instruction[i]);
        if(it1!=leader.end())
            isleader[i]=true;  //If an instruction is a leader, mark isleader of this instruction true.
        if(it2!=branch.end() && (i<isleader.size()-1))
            isleader[i+1]=true;  //If an instruction is a branch, mark isleader of the next instruction true.
    }
}

//Identify basic blocks and count the execution time.
void find_block(){
    map<unsigned long long, BLOCK>::iterator it;
    unsigned long long start=instruction[0]; //The start of a basic block.
    unsigned int counter=1;                  //Record the number of instructions in each block.
    unsigned int id=0;
    BLOCK temp={0};

    //Iterate through isleader.
    for(unsigned int i=1; i<isleader.size(); i++){
        if(isleader[i]){
            it=block_info.find(start);
            if(it==block_info.end()){       //If an instruction is a leader and not identified yet,
                temp.num_instr=counter;     //create a new basic block structure.
                temp.stop=instruction[i-1];
                temp.id=id;
                temp.num_use=1;
                block_info[start]=temp;
                id++;
            }
            else{                           //If an instruction is a leader and has been identified,
                (it->second).num_use++;     //increment the number of execution.
            }
            counter=1;
            start=instruction[i];
        }
        else                               //If an instruction is not a leader
            counter++;                     //increase the number of instructions in the block.
    }
    //Deal with the last instruction.
    if(isleader.back()){
        it=block_info.find(instruction.back());
        if(it==block_info.end()){
            temp.num_instr=1;
            temp.stop=instruction.back();
            temp.id=id;
            temp.num_use=1;
            block_info[instruction.back()]=temp;
        }
        else
            (it->second).num_use++;
    }
    else{
            it=block_info.find(start);
            if(it==block_info.end()){
                temp.num_instr=counter;
                temp.stop=instruction.back();
                temp.id=id;
                temp.num_use=1;
                block_info[start]=temp;
                id++;
            }
            else{
                (it->second).num_use++;
            }
    }
}

//Identify control flows and count the branching frequency.
void find_flow(){
    map<unsigned long long, BLOCK>::iterator it1;
    map<FLOW, unsigned int, comp>::iterator it2;
    unsigned int from_id=0;
    FLOW temp={0};

    //Iterate through isleader.
    for(unsigned int i=1; i<isleader.size(); i++){
        if(isleader[i]){
            temp.from_id=from_id;
            it1=block_info.find(instruction[i]);
            unsigned int to_id=(it1->second).id;
            temp.to_id=to_id;                       //Map starting block to ending block.
            from_id=to_id;

            it2=flow_info.find(temp);
            if(it2!=flow_info.end())                //If flow info has been identified,
                (it2->second)++;                    //increment the usage.
            else                                    //If flow info has not been identified,
                flow_info[temp]=1;                  //create a new one and set usage to 1.
        }
    }
}

//Write basic block information to file.
void saveBlockInfo(string s){
    ofstream outfile;
    size_t found = s.find_last_of("/\\");
    string filename=s.substr(found+1)+".block.txt";
    outfile.open(filename.c_str());
    map<unsigned long long, BLOCK>::iterator it;
    for(it=block_info.begin(); it!=block_info.end(); it++){
        outfile<<"bb"<<(it->second).id<<"\t"<<hex<<(it->first)<<"\t"<<dec<<(it->second).num_instr<<"\t"<<(it->second).num_use<<endl;
    }
}

//Write control flow informatin to file.
void saveFlowInfo(string s){
    ofstream outfile;
    size_t found = s.find_last_of("/\\");
    string filename=s.substr(found+1)+".flow.txt";
    outfile.open(filename.c_str());
    map<FLOW, unsigned int, comp>::iterator it, temp1;
    for(it=flow_info.begin(); it!=flow_info.end(); it++){
        outfile<<"bb"<<(it->first).from_id<<"\t"<<"bb"<<(it->first).to_id<<"\t"<<(it->second)<<endl;
    }
}

int main(int argc, char* argv[])
{
    if(argc!=2){
        cout<<"Wrong number of arguments."<<endl;
        return 0;
    }

    ifstream infile(argv[1]);
    if(!infile){
        cout<<"file does not exist."<<endl;
        return 0;
    }

 /*   ifstream infile("/home/zhe/Documents/simple.trace");
        if(!infile){
        cout<<"file does not exist."<<endl;
        return 0;}*/
    unsigned long long next_addr=-1;
    string line;
    while(getline(infile, line)){
        if(line[0]=='I'){
            unsigned long long address;
            unsigned int length;
            parseInstruction(line, &address, &length);
            if(next_addr!=-1 && next_addr!=address){                //If the sum of current instruction address and its size
                unsigned long long last_addr=instruction.back();    //does not match the next instruction address,
                branch[last_addr]=1;                                //the current instruction is a branch
                leader[address]=1;                                  //and the next instruction is a leader.
            }
            instruction.push_back(address);
            next_addr=address+length;
        }
    }

    if(instruction.empty()){
        cout<<"No type 'I' instruction found. Please check your input file."<<endl;
        return 0;
    }
    isleader.resize(instruction.size(), false);
    mark_leader();
    find_block();
    //cout<<block_info.size()<<endl;
    find_flow();
    saveBlockInfo(argv[1]);
    saveFlowInfo(argv[1]);

    return 0;
}

