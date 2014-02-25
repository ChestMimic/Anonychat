INC=-I../ac-lib/
LIB=../ac-lib
GCC=gcc $(INC)

all: name_server

name_server: name_server.o $(LIB)/list.o
	$(GCC) name_server.o $(LIB)/list.o -o name_server -pthread
	
name_client: nameclient.o nameclient_test.o
	$(GCC) nameclient_test.o nameclient.o -o name_client
	
name_server.o: name_server.c name_server.h
	$(GCC) -c name_server.c -pthread

name_client.o: nameclient.c nameclient.h
	$(GCC) -c nameclient.c
	
nameclient_test.o: nameclient_test.c
	$(GCC) -c nameclient_test.c
	
clean:
	rm *.o name_server name_client
