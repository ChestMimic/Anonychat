INC=-I../ac-lib/
LIB=../ac-lib
GCC=gcc $(INC)

all: name_server

name_server: name_server.o $(LIB)/list.o $(LIB)/client.o
	$(GCC) $^ -o name_server -pthread
	
name_server.o: name_server.c name_server.h
	$(GCC) -c name_server.c -pthread
		
clean:
	rm -f *.o name_server
