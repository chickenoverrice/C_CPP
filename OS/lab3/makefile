mmu : main.o pte.o replacement.o 
	g++ -o mmu main.o pte.o replacement.o 

main.o : main.cpp 
	g++ -Wall -fexceptions -g -Iinclude -c main.cpp
pte.o : pte.cpp pte.h 
	g++ -Wall -fexceptions -g -Iinclude -c pte.cpp
replacement.o : replacement.cpp replacement.h 
	g++ -Wall -fexceptions -g -Iinclude -c replacement.cpp
clean :
	rm mmu main.o pte.o replacement.o
