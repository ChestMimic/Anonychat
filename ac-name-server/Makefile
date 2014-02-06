all: name_server name_client

name_server: name_server.o
	gcc name_server.o -o name_server
	
name_server.o: name_server.c name_server.h
	gcc -c name_server.c

name_client: name_client.o
	gcc nameclient.o -o name_client

name_client.o: nameclient.c
	gcc -c nameclient.c
	
clean:
	rm *.o name_server name_client
