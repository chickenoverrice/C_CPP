process_scheduler : main.o process.o scheduler.o 
	g++ -o process_scheduler main.o process.o scheduler.o 

main.o : main.cpp 
	g++ -c main.cpp
process.o : process.cpp process.h 
	g++ -c process.cpp
scheduler.o : scheduler.cpp scheduler.h 
	g++ -c scheduler.cpp
clean :
	rm process_scheduler main.o process.o scheduler.o 
