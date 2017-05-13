/* Author: Zhe Xu
 * Course: Operating Systems Fall16
 * Assignment: Linker
 * Date: 09/22/2016
 */

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <list>
#include <queue>
#include <vector>
#include <iterator>

using namespace std;

struct TOKEN{
	char s[80];
	int offset;
	int line_num;
};

struct SYMBOL{
	char s[80];
	int addr;
	int module;
	int error;
	int warning;
};

struct INSTRUCTION{
	char c;
	int addr;
	int module;
    int error;
	int warning;
};

queue<TOKEN> token_list;
list<SYMBOL> symbol_list;
list<SYMBOL> use_list;
vector<INSTRUCTION> instr_list;
vector<int> module_list;
vector<int> module_use_list;

int MACHINE_SIZE = 512;
int SYMBOL_LEN = 16;

/*
 * Function: __parseerror
 * prints error message.
 * linenum: linenum of a token; lineoffset: line offset of a token; errocode: error code
 */
void __parseerror(int linenum, int lineoffset, int errcode) {
    const char* errstr[] = {
        "NUM_EXPECTED", // Number expect
        "SYM_EXPECTED", // Symbol Expected
        "ADDR_EXPECTED", // Addressing Expected
        "SYM_TOLONG", // Symbol Name is to long
        "TO_MANY_DEF_IN_MODULE", // > 16
        "TO_MANY_USE_IN_MODULE",  // > 16
        "TO_MANY_INSTR",  //total num_instr exceeds memory size (512)
        "SYM_EXPECTED_INVALID_SYM_NAME"
    };
    printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode]);
}

/*
 * Function: print_warning_7
 * prints warning message rule7.
 */
void print_warning_7(int module){
    list<SYMBOL>::iterator it;
    for(it = use_list.begin(); it != use_list.end(); it++){
        if((it->warning == 7) && (module == it->module))
            printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", it->module, it->s);
            }
    return;
}

/*
 * Function: print_warning_5
 * prints warning message rule5.
 */
void print_warning_5(int module){
    list<SYMBOL>::iterator it;

    for(it = symbol_list.begin(); it != symbol_list.end(); it++){
        if(((int)(it->warning / 1000) == 5) && (module == it->module))
            printf("Warning: Module %d: %s to big %d (max=%d) assume zero relative\n", it->module, it->s, (it->warning)%1000, module_list[it->module]-1);
    }
    return;
}

void print_instr_1(){
    unsigned int i;

	for(i = 0; i < instr_list.size(); i++){
        if(i == instr_list.size()-1)
            print_warning_5(instr_list[i].module);
		else{
            if(instr_list[i].module != instr_list[i+1].module)
                print_warning_5(instr_list[i].module);
		}
    }
}

/*
 * Function: print_symbol
 * prints Symbol Table and possible error messages.
 */
void print_symbol(){
	list<SYMBOL>::iterator it;
	printf("Symbol Table\n");
	for(it = symbol_list.begin();it != symbol_list.end(); it++){
        switch(it->error){
            case 2:
                printf("%s=%d Error: This variable is multiple times defined; first value used\n", it->s, it->addr);
                break;
            case 21:
                break;
            default:
                printf("%s=%d\n", it->s, it->addr);
        }
	}
	puts(" ");
}

/*
 * Function: print_instruction
 * prints Memory Map and possible error messages.
 */
void print_instruction(){
	unsigned int i;
	list<SYMBOL>::iterator it1;

    int item = 0;

	printf("Memory Map\n");

	for(i = 0; i < instr_list.size(); i++){
        int count = -1;
        switch(instr_list[i].error){
            case 3:{
                for(it1 = use_list.begin(); it1 != use_list.end(); it1++){
                    if(it1->module == instr_list[i].module){
                        count++;
                    if(count == instr_list[i].addr%1000)
                        break;
                    }
                }
                printf("%03d:  %04d Error: %s is not defined; zero used\n", item, instr_list[i].addr, it1->s);
                break;
                }
            case 6:
                printf("%03d:  %04d Error: External address exceeds length of uselist; treated as immediate\n", item, instr_list[i].addr);
                break;
            case 8:
                printf("%03d:  %04d Error: Absolute address exceeds machine size; zero used\n", item, instr_list[i].addr);
                break;
            case 9:
                printf("%03d:  %04d Error: Relative address exceeds module size; zero used\n", item, instr_list[i].addr);
                break;
            case 10:
                printf("%03d:  %04d Error: Illegal immediate value; treated as 9999\n", item, instr_list[i].addr);
                break;
            case 11:
                printf("%03d:  %04d Error: Illegal opcode; treated as 9999\n", item, instr_list[i].addr);
                break;
            default:
                printf("%03d:  %04d\n", item, instr_list[i].addr);
        }
		item++;
		if(i == instr_list.size()-1)
            print_warning_7(instr_list[i].module);
		else{
            if(instr_list[i].module != instr_list[i+1].module)
                print_warning_7(instr_list[i].module);
		}
	}
	puts(" ");
}


/*
 * Function: print_warning
 * prints warning messages.
 */
void print_warning(){
    list<SYMBOL>::iterator it;
    for(it = symbol_list.begin(); it != symbol_list.end(); it++){
        if(it->warning == 4)
            printf("Warning: Module %d: %s was defined but never used\n", it->module, it->s);
    }
    return;
}

void print_used_symbol(){
	list<SYMBOL>::iterator it;
	printf("Used Symbol Table:\n");
	for(it = use_list.begin();it != use_list.end(); it++){
		printf("%s  %d  %d\n", it->s, it->addr, it->module);
	}
}

void print_module(){
    for(unsigned i=0; i<module_list.size();i++)
        printf("%d ", module_list[i]);
}

/*
 * Function: tokenize
 * reads tokens from a given file into the token_list.
 * filename: the string of a file directory.
 * returns: -1 if function fails, 1 otherwise.
 */
int tokenize(const char* filename){
	FILE* fh;
	TOKEN temp={0};
	char s[] = " \t\n";
    char *token;
	int offset = 0;
	int line_num = 0;

	fh = fopen(filename, "r");

	if (fh == NULL){
    printf("File does not exist.\n");
    return -1;
	}

	const size_t line_size = 300;
	char* line = (char*)malloc(line_size);
	while (fgets(line, line_size, fh) != NULL)  {
		    token = strtok(line, s);
			line_num++;
		while( token != NULL ) {
				strcpy(temp.s,token);
				offset = token-line + 1;
				temp.offset = offset;
				temp.line_num = line_num;
				token_list.push(temp);
				token = strtok(NULL, s);
				}

		}
	free(line);
	fclose(fh);
	return 1;

}

/*
 * Function: valid_symbol_name
 * validates whether a given string is an accepted symbol name.
 * the rule of a valid symbol name is described in Assignment.
 * name: the name of a symbol.
 * returns: 1 if valid, 0 otherwise.
 */
int valid_symbol_name(const char* name){
    if(isdigit(name[0])) return 0;
    for(unsigned i = 1; i < strlen(name); i++){
        if(!isalnum(name[i]))
            return 0;
    }
    return 1;
}

/*
 * Function: valid_value
 * validates whether a given string is a value.
 * value: the string representing a possible value.
 * returns: 1 if valid, 0 otherwise.
 */
int valid_value(const char* value){
    for(unsigned i = 0; i < strlen(value); i++){
        if(!isdigit(value[i]))
            return 0;
    }
    return 1;
}

/*
 * Function: pass1
 * parses tokens, generates metadata for symbols, instructions and modules,
 * and reports certain errors (as required in Assignment).
 * filename: the string of a file directory.
 * returns: -1 if an error occurs, 1 if pass is success.
 */
int pass1(const char* filename){
	int t = tokenize(filename);
	if(t == -1) return -1; //file does not exist.

	if(token_list.empty()){
        __parseerror(1, 1, 0);
        return -1;
	}                 //empty file.

	int defnum =  0;
	int usenum = 0;
	int instrnum = 0;
	int module = 1;
	int total_instr = 0;
	int temp_line = 0;
	int temp_off = 0;

	module_list.push_back(0);
	module_use_list.push_back(0);
	SYMBOL temp1 = {0};
	INSTRUCTION temp2 = {0};

	while (!token_list.empty()){

        /* read symbol definitions */
        if(!valid_value(token_list.front().s)){
            __parseerror(token_list.front().line_num, token_list.front().offset, 0);
            return -1;
        }
		defnum = atoi(token_list.front().s);
		if(defnum > 16){
            __parseerror(token_list.front().line_num, token_list.front().offset, 4);
            return -1;
		}
		temp_line = token_list.front().line_num;
		temp_off = token_list.front().offset + strlen(token_list.front().s);

		token_list.pop();

		for(int i = 0; i < defnum; i++){
            if(token_list.empty()){
                __parseerror(temp_line, temp_off, 1);
                return -1;
            }
            if(strlen(token_list.front().s)>16){
                __parseerror(token_list.front().line_num, token_list.front().offset, 3);
                return -1;
            }

            if(!valid_symbol_name(token_list.front().s)){
                __parseerror(token_list.front().line_num, token_list.front().offset, 7);
                return -1;
            }

            strcpy(temp1.s, token_list.front().s);
            temp_line = token_list.front().line_num;
            temp_off = token_list.front().offset + strlen(token_list.front().s);
			token_list.pop();
            if(token_list.empty()){
                __parseerror(temp_line, temp_off, 0);
                return -1;
            }
			if(!valid_value(token_list.front().s)){
                __parseerror(token_list.front().line_num, token_list.front().offset, 0);
                return -1;
            }
			temp1.addr = atoi(token_list.front().s);
            temp_line = token_list.front().line_num;
            temp_off = token_list.front().offset + strlen(token_list.front().s);
			token_list.pop();
			temp1.module = module;
			temp1.error = temp1.warning = 0;
			symbol_list.push_back(temp1);
		}

        /* read used symbol */
        if(token_list.empty()){
            __parseerror(temp_line, temp_off, 0);
            return -1;
        }
        if(!valid_value(token_list.front().s)){
            __parseerror(token_list.front().line_num, token_list.front().offset, 0);
            return -1;
            }
		usenum = atoi(token_list.front().s);
		temp_line = token_list.front().line_num;
		temp_off = token_list.front().offset + strlen(token_list.front().s);

        if(usenum > 16){
            __parseerror(token_list.front().line_num, token_list.front().offset, 5);
            return -1;
		}
		token_list.pop();
        if(token_list.empty()){
            __parseerror(temp_line, temp_off, 1);
            return -1;
        }
		for(int i = 0; i < usenum; i++){
            if(token_list.empty()){
                __parseerror(temp_line, temp_off, 1);
                return -1;
            }
            if(!valid_symbol_name(token_list.front().s)){
                __parseerror(token_list.front().line_num, token_list.front().offset, 1);
                return -1;
            }
            strcpy(temp1.s, token_list.front().s);
            temp_line = token_list.front().line_num;
            temp_off = token_list.front().offset + strlen(token_list.front().s);
			token_list.pop();
			temp1.addr = -1;
			temp1.module = module;
			temp1.error = temp1.warning = 0;
			use_list.push_back(temp1);
		}
		module_use_list.push_back(usenum);

        /* read instructions */
        if(token_list.empty()){
            __parseerror(temp_line, temp_off, 0);
            return -1;
        }
        if(!valid_value(token_list.front().s)){
            __parseerror(token_list.front().line_num, token_list.front().offset, 0);
            return -1;
            }
		instrnum = atoi(token_list.front().s);

		temp_line = token_list.front().line_num;
		temp_off = token_list.front().offset + strlen(token_list.front().s);

        total_instr += instrnum;
		if(total_instr > 512){
            __parseerror(token_list.front().line_num, token_list.front().offset, 6);
            return -1;
		}

		token_list.pop();
		if(token_list.empty()){
            __parseerror(temp_line, temp_off, 2);
            return -1;
        }

		for(int i = 0; i < instrnum; i++){
            if(token_list.empty()){
                __parseerror(temp_line, temp_off, 2);
                return -1;
            }
			temp2.c = token_list.front().s[0];
            temp_line = token_list.front().line_num;
            temp_off = token_list.front().offset + strlen(token_list.front().s);

			token_list.pop();
            if(token_list.empty()){
                __parseerror(temp_line, temp_off, 0);
                return -1;
            }
            if(!valid_value(token_list.front().s)){
                __parseerror(token_list.front().line_num, token_list.front().offset, 0);
                return -1;
            }

			temp2.addr = atoi(token_list.front().s);
			if(temp2.addr < 1000 && temp2.addr){
                __parseerror(token_list.front().line_num, token_list.front().offset, 2);
                return -1;
			}

            temp_line = token_list.front().line_num;
            temp_off = token_list.front().offset + strlen(token_list.front().s);
			token_list.pop();
			temp2.module = module;
			temp2.error = temp2.warning = 0;
			instr_list.push_back(temp2);
		}
		module_list.push_back(instrnum);
		module++;
	}

    for(unsigned i = 1; i < module_list.size(); i++)
        module_list[i] += module_list[i-1];

	list<SYMBOL>::iterator it;

	for(it = symbol_list.begin();it != symbol_list.end(); it++){
        it->addr += module_list[it->module-1];
        if((it->addr)%1000 > module_list[it->module]){
            it->warning = 5000 + (it->addr)%1000;
            it->addr = module_list[it->module-1];
            }
        }

	return 1;
}

/*
 * Function: remove_duplicate
 * removes duplicated symbols while storing them into the symbol_list.
 */
void remove_duplicate(){
    list<SYMBOL>::iterator it1, it2;

    for(it1 = symbol_list.begin();it1 != symbol_list.end(); it1++){
        it2 = it1;
        advance(it2, 1);
        while(it2 != symbol_list.end()){
            if(!strcmp(it1->s, it2->s)){
                it1->error = 2;
                it2->error = 21;
            }
            it2++;
        }
    }
    return;
}

/*
 * Function: find_external
 * finds the absolute address of an external symbol.
 * addr: the relative address of the external symbol;
 * module: the module of the external symbol.
 * returns: -1 if the symbol is not found;
 * otherwise returns the address of the external symbol.
 */
int find_external(int addr, int module){
    list<SYMBOL>::iterator it1, it2;

    int count = -1;

    for(it1 = use_list.begin(); it1 != use_list.end(); it1++){
        if(it1-> module == module){
            count++;
            if(count == addr%1000)
                break;
        }
    }

    for(it2 = symbol_list.begin(); it2 != symbol_list.end(); it2++){
        if(!strcmp(it1->s, it2->s))
            return it2->addr;

    }

    return -1;
    }

/*
 * Function: pass2
 * relocates relative address, resolves external reference,
 * and reports certain errors (as required in Assignment).
 */
void pass2(){
    vector<INSTRUCTION>::iterator it;
    list<SYMBOL>::iterator s_it;
    list<SYMBOL>::iterator u_it;

    int module = 1;
    int count = 0;
    bool used = false;

    for(s_it = symbol_list.begin(); s_it != symbol_list.end(); s_it++){
        for(u_it = use_list.begin(); u_it != use_list.end(); u_it++){
            if(!strcmp(s_it->s, u_it->s))
                used = true;
        }
        if(!used)
            s_it->warning = 4;
        used = false;
    }

    for(u_it = use_list.begin(); u_it != use_list.end(); u_it++){
        if(module != u_it->module){
            count = 0;
            module = u_it->module;
        }
        for(it = instr_list.begin(); it != instr_list.end(); it++){
            if(u_it->module == it->module && it->c == 'E' && (it->addr)%1000== count){
                used = true;
                break;
            }
        }
        count++;
        if(used == false)
            u_it->warning = 7;
        used = false;
    }

	for(it = instr_list.begin(); it != instr_list.end(); it++){
        if(it->addr > 9999){
            it->addr = 9999;
            if(it->c == 'I')
                it->error = 10;
                else it->error = 11;
        }
        else{
            if(it->c == 'A' && (it->addr%1000) > MACHINE_SIZE){
                it->error = 8;
                it->addr = it->addr - it->addr%1000;
            }
            if(it->c == 'R'){
                if((it->addr%1000) > module_list[it->module] - module_list[it->module-1]){
                    it->error = 9;
                    it->addr = it->addr - it->addr%1000 + module_list[it->module-1];
                }
                else
                    it->addr += module_list[it->module-1];
                }

            if(it->c == 'E'){
                if((it->addr%1000) > module_use_list[it->module] - 1){
                    it->error = 6;
                    it->c = 'R';
                    }
                else{
                    int i = find_external(it->addr, it->module);
                    if(i == -1){
                        it->error = 3;
                        it->addr = it->addr - (it->addr%1000);
                    }
                    else it->addr = it->addr - (it->addr%1000) + i;
                }
            }
        }
	}


	return;
}


int main(int argc, char* argv[])
{
    if(argc < 2){
        puts("missing argument");
        return 0;
        }

    int err = 0;
    //err = pass1("/home/zhe/Documents/input-8");
    err = pass1(argv[argc-1]);
	if(err == -1) return 0;

    remove_duplicate();

	pass2();
	print_instr_1();
    print_symbol();
	print_instruction();
	print_warning();

	return 0;
}
