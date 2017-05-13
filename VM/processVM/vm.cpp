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
typedef int (*pfunc)(string, string);

map<string, pfunc> funcMap;
map<string, int> regMap;
map<string, int> labelMap;
map<string, int> addressMap;

vector<string> instruction;
vector<int> vmStack;
vector<int> vmRegister(17,0);
vector<unsigned int> counter;
vector<unsigned int> labelPos;
vector<bool> branch;

char testArg(string s){
    char res;
    if(isdigit(s[0]))
        res='v';
    else if(s[0]=='[')
        res='a';
    else
        res='r';
    return res;
}

int convertNum(string s){
    int res;
    int len=s.length();
    char* pEnd;
    if(s[0]=='0' && s[1]=='x'){
        string temp=s.substr(2,len-2);
        char * dup = strdup(temp.c_str());
        res=(int)strtol(dup, &pEnd, 16);
    }
    else{
        if(s[len-1]=='h'){
            string temp=s.substr(0,len-2);
            char * dup = strdup(temp.c_str());
            res=(int)strtol(dup, &pEnd, 16);
        }
        else if(s[len-1]=='b'){
            string temp=s.substr(0,len-2);
            char * dup = strdup(temp.c_str());
            res=(int)strtol(dup, &pEnd, 2);
        }
        else{
            char * dup = strdup(s.c_str());
            res=(int)strtol(dup, &pEnd, 10);
        }
    }
    return res;
}

int findNumber(string s){
    map<string, int>::iterator it;
    it=addressMap.find(s);
    if(it!=addressMap.end())
        return (it->second);
    else
        return 0;
}

int mov(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
            adder=findNumber(arg2);
    }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]=adder;
        }
    else{
        addressMap[arg1]=adder;
    }
    vmRegister[8]++;
    return 0;
}

int push(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='v')
        vmStack.push_back(convertNum(arg1));
    else if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmStack.push_back(vmRegister[it->second]);
    }
    else{
            vmStack.push_back(findNumber(arg1));
    }
    vmRegister[8]++;
    vmRegister[6]=vmStack.size()-1;
    return 0;
}

int pop(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]=vmStack.back();
        vmStack.pop_back();
    }
    else{
            addressMap[arg1]=vmStack.back();
            vmStack.pop_back();
    }
    vmRegister[8]++;
    vmRegister[6]=vmStack.size()-1;
    return 0;
}

int pushf(string arg1="", string arg2=""){
    vmStack.push_back(vmRegister[4]);
    vmRegister[8]++;
    vmRegister[6]=vmStack.size()-1;
    return 0;
}

int popf(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]=vmRegister[4];
    }
    else{
        addressMap[arg1]=vmRegister[4];
        vmRegister[6]=vmStack.size()-1;
    }
    vmRegister[8]++;
    vmRegister[4]=0;
    return 0;
}

int call(string arg1, string arg2=""){
    vmStack.push_back(vmRegister[8]);
    branch[vmRegister[8]]=true;
    vmRegister[6]=vmStack.size()-1;//base points to the last address of the caller.
    //vmRegister[7]=vmStack.size()-1;
    map<string, int>::iterator it;
    it=labelMap.find(arg1);
    vmRegister[8]=(it->second);
    return 0;
}

int ret(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    vmRegister[8]=vmStack.back();
    vmStack.pop_back();
    vmRegister[6]=vmStack.size()-1;
    vmRegister[8]++;
    return 0;
}

int inc(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]++;
    }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)++;
        else
            addressMap[arg1]=1;
    }
    vmRegister[8]++;
    return 0;
}

int dec(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]--;
    }
    else{
       map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)--;
        else
            addressMap[arg1]=-1;
    }
    vmRegister[8]++;
    return 0;
}

int add(string arg1, string arg2){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
        }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]+=adder;
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)+=adder;
        else
            addressMap[arg1]=adder;
        }
    vmRegister[8]++;
    return 0;
}

int sub(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
        }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]-=adder;
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)-=adder;
        else
            addressMap[arg1]=-adder;
        }
    vmRegister[8]++;
    return 0;
}

int mul(string arg1, string arg2=""){
   char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
        }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]*=adder;
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)*=adder;
        else
            addressMap[arg1]=0;
        }
    vmRegister[8]++;
    return 0;
}

int div(string arg1, string arg2=""){
   char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
        }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]/=adder;
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)/=adder;
        else
            addressMap[arg1]=0;
        }
    vmRegister[8]++;
    return 0;
}

int mod(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
    }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[5]=vmRegister[it->second]%adder;
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            vmRegister[5]=(it->second)%adder;
        else
            vmRegister[5]=0%adder;
    }
    vmRegister[8]++;
    return 0;
}

int rem(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]=vmRegister[5];
    }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)=vmRegister[5];
        else
            addressMap[arg1]=vmRegister[5];
    }
    vmRegister[5]=0;
    vmRegister[8]++;
    return 0;
}

int notf(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]=~(vmRegister[it->second]);
    }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)=~(it->second);
        else
            addressMap[arg1]=1;
    }
    vmRegister[8]++;
    return 0;
}

int xorf(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
            adder=findNumber(arg2);
        }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]^=adder;
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)^=adder;
        else
            addressMap[arg1]=(0^adder);
    }
    vmRegister[8]++;
    return 0;
}

int orf(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
    }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]|=adder;
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)|=adder;
        else
            addressMap[arg1]=(0|adder);
    }
    vmRegister[8]++;
    return 0;
}

int andf(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
    }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]&=adder;
        }
    else{
    map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)&=adder;
        else
            addressMap[arg1]=(0&adder);
        }
    vmRegister[8]++;
    return 0;
}

int shl(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
    }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]=(vmRegister[it->second]<<adder);
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)=((it->second)<<adder);
        else
            addressMap[arg1]=0;
    }
    vmRegister[8]++;
    return 0;
}

int shr(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
    }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        vmRegister[it->second]=(vmRegister[it->second]>>adder);
        }
    else{
        map<string, int>::iterator it;
        it=addressMap.find(arg1);
        if(it!=addressMap.end())
            (it->second)=((it->second)>>adder);
        else
            addressMap[arg1]=0;
        }
    vmRegister[8]++;
    return 0;
}

int cmp(string arg1, string arg2=""){
    char c1=testArg(arg1);
    char c2=testArg(arg2);
    int adder;
    if(c2=='r'){
        map<string, int>::iterator it2;
        it2=regMap.find(arg2);
        adder=vmRegister[it2->second];
    }
    else if(c2=='v'){
        adder=convertNum(arg2);
    }
    else{
        adder=findNumber(arg2);
    }
    if(c1=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        if(vmRegister[it->second]>adder)
            vmRegister[4]=1;
        else if(vmRegister[it->second]==adder)
            vmRegister[4]=0;
        else
            vmRegister[4]=-1;
        }
    else if(c1=='v'){
        int compare=convertNum(arg1);
        if(compare>adder)
            vmRegister[4]=1;
        else if(compare==adder)
            vmRegister[4]=0;
        else
            vmRegister[4]=-1;
    }
    else{
        int compare=findNumber(arg2);
        if(compare>adder)
            vmRegister[4]=1;
        else if(compare==adder)
            vmRegister[4]=0;
        else
            vmRegister[4]=-1;
    }
    vmRegister[8]++;
    return 0;
}

int jmp(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    map<string, int>::iterator it;
    it=labelMap.find(arg1);
    vmRegister[8]=it->second;
    return 0;
}

int je(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    if(vmRegister[4]==0){
        map<string, int>::iterator it;
        it=labelMap.find(arg1);
        vmRegister[8]=it->second;
    }
    else
        vmRegister[8]++;
    return 0;
}

int jne(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    if(vmRegister[4]!=0){
        map<string, int>::iterator it;
        it=labelMap.find(arg1);
        vmRegister[8]=it->second;
    }
    else
        vmRegister[8]++;
    return 0;
}

int jg(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    if(vmRegister[4]>0){
        map<string, int>::iterator it;
        it=labelMap.find(arg1);
        vmRegister[8]=it->second;
    }
    else
        vmRegister[8]++;
    return 0;
}

int jge(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    if(vmRegister[4]>=0){
        map<string, int>::iterator it;
        it=labelMap.find(arg1);
        vmRegister[8]=it->second;
    }
    else
        vmRegister[8]++;
    return 0;
}

int jl(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    if(vmRegister[4]<0){
        map<string, int>::iterator it;
        it=labelMap.find(arg1);
        vmRegister[8]=it->second;
    }
    else
        vmRegister[8]++;
    return 0;
}

int jle(string arg1, string arg2=""){
    branch[vmRegister[8]]=true;
    if(vmRegister[4]<=0){
        map<string, int>::iterator it;
        it=labelMap.find(arg1);
        vmRegister[8]=it->second;
    }
    else
        vmRegister[8]++;
    return 0;
}

int prn(string arg1, string arg2=""){
    char c=testArg(arg1);
    if(c=='v')
        cout<<convertNum(arg1)<<endl;
    else if(c=='r'){
        map<string, int>::iterator it;
        it=regMap.find(arg1);
        cout<<vmRegister[it->second]<<endl;
    }
    else{
        cout<<findNumber(arg1)<<endl;
    }
    vmRegister[8]++;
    return 0;
}

void createMap(){
    funcMap["mov"]=&mov;
    funcMap["push"]=&push;
    funcMap["pop"]=&pop;
    funcMap["pushf"]=&pushf;
    funcMap["popf"]=&popf;
    funcMap["call"]=&call;
    funcMap["ret"]=&ret;
    funcMap["inc"]=&inc;
    funcMap["dec"]=&dec;
    funcMap["add"]=&add;
    funcMap["sub"]=&sub;
    funcMap["mul"]=&mul;
    funcMap["div"]=&div;
    funcMap["mod"]=&mod;
    funcMap["rem"]=&rem;
    funcMap["not"]=&notf;
    funcMap["or"]=&orf;
    funcMap["and"]=&andf;
    funcMap["xor"]=&xorf;
    funcMap["shl"]=&shl;
    funcMap["shr"]=&shr;
    funcMap["cmp"]=&cmp;
    funcMap["jmp"]=&jmp;
    funcMap["je"]=&je;
    funcMap["jne"]=&jne;
    funcMap["jg"]=&jg;
    funcMap["jge"]=&jge;
    funcMap["jl"]=&jl;
    funcMap["jle"]=&jle;
    funcMap["prn"]=&prn;
    regMap["eax"]=0;
    regMap["ebx"]=1;
    regMap["ecx"]=2;
    regMap["edx"]=3;
    regMap["esi"]=4;//flag
    regMap["edi"]=5;//mod
    regMap["esp"]=6;//stack
    regMap["ebp"]=7;//base
    regMap["eip"]=8;//instruction
    regMap["r08"]=9;
    regMap["r09"]=10;
    regMap["r10"]=11;
    regMap["r11"]=12;
    regMap["r12"]=13;
    regMap["r13"]=14;
    regMap["r14"]=15;
    regMap["r15"]=16;
}

vector<string> parseInstruction(string s){
    vector<string> split;//cout<<s<<endl;
    s.erase(s.find_last_not_of(" \n\r\t\v\f")+1);
    char * dup = strdup(s.c_str());
    char * token = strtok(dup, " ,\t");
    while(token != NULL){
        //cout<<token<<endl;
        split.push_back(string(token));
        token = strtok(NULL, " ,");
    }
    free(dup);
    return split;
}

void printInstr(){
    for(int i=0; i<instruction.size(); i++)
        cout<<i<<"  "<<instruction[i]<<"  "<<counter[i]<<endl;
}

void printLabel(){
    map<string, int>::iterator it;
    for(it=labelMap.begin(); it!=labelMap.end(); it++)
        cout<<it->first<<" "<<it->second<<endl;
    for(unsigned int i=0; i<labelPos.size(); i++)
        cout<<labelPos[i]<<endl;
}

void saveStat(string s){
    ofstream outfile;
    string filename=s+".txt";
    outfile.open(filename.c_str());

    outfile<<counter[0]<<endl;
    instruction[0].erase(0,instruction[0].find_first_not_of(" \t"));
    outfile<<instruction[0]<<endl;
    for(unsigned int i=1; i<branch.size(); i++){
        if(branch[i-1]==true){
            outfile<<endl;
            outfile<<counter[i]<<endl;
        }
        instruction[i].erase(0,instruction[i].find_first_not_of(" \t"));;
        outfile<<instruction[i]<<endl;
    }
}

void runProgram(){
    map<string, int>::iterator it;
    it=labelMap.find("start");
    if(it!=labelMap.end())
        vmRegister[8]=(it->second);
    while(vmRegister[8]<instruction.size()){
        int in=vmRegister[8]; //cout<<instruction[in]<<endl;
        counter[in]++;
        vector<string> op=parseInstruction(instruction[in]);
        string arg1="";
        string arg2="";
        if(op.size()==3){
            arg1=op[1];
            arg2=op[2];
        }
        else if(op.size()==2)
            arg1=op[1];
        map<string, pfunc>::iterator it=funcMap.find(op[0]);
        (*it->second)(arg1, arg2);
    }
}

void printCounter(){
    for(unsigned int i=0; i<counter.size(); i++)
        cout<<counter[i]<<endl;
}

int main(int argc, char* argv[])
{
    ifstream infile(argv[1]);
    //ifstream infile;
    //infile.open("/home/zhe/Documents/NYU_VM/hw2/test2.vm");
    //string progname="loop.vm";
    string line;
    int linenumber=0;
    while(getline(infile, line)){
        if(line[0]=='#')
            continue;
        if(line[line.length()-1]=='\r')
            line.erase(line.length()-1);
        size_t notempty=line.find_last_not_of(" \t\f\v\n\r");
        if(notempty==string::npos)
            continue;
        if(line.empty())
            continue;
        size_t i=line.find(":");
        size_t j=line.find("#");
        string line2;
        if(j!=string::npos)
            line2=line.substr(0,j);
        else
            line2=line;
        string label;
        string remain;
        if(i!=string::npos){
            label=line2.substr(0,i);
            size_t last=line2.find_last_not_of(" \t\f\v\n\r");
            labelPos.push_back(linenumber);
            if(i==last){
                labelMap[label]=linenumber;
            }
            else{
                labelMap[label]=linenumber;
                remain=line2.substr(i+1,line2.length()-i-1);
                instruction.push_back(remain);
                linenumber++;
            }
        }
        else{
            instruction.push_back(line2);
            linenumber++;
        }
    }

    counter.resize(instruction.size(),0);
    branch.resize(instruction.size(), false);
    for(unsigned int i=0; i<labelPos.size(); i++){
        if(labelPos[i]>0)
            branch[labelPos[i]-1]=true;
    }
    createMap();
    runProgram();
    saveStat(argv[1]);

    return 0;
}

