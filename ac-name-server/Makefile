all: name_server

name_server: name_server.o
	gcc name_server.o -o name_server
	
name_server.o: name_server.c name_server.h
	gcc -c name_server.c
	
clean:
	rm *.o name_server