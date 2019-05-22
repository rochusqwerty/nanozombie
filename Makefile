all: bank

bank: main.o init.o
	mpicc main.o init.o -o bank

init.o: init.cpp 
	mpicc init.cpp -c -Wall

main.o: main.cpp main.h
	mpicc main.cpp -c -Wall

clear: 
	rm *.o bank
