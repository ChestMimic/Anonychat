all: name_server name_client

name_server: name_server.o client_list.o
	gcc name_server.o client_list.o -o name_server
	
name_client: name_client.o
	gcc nameclient.o -o name_client
	
name_server.o: name_server.c name_server.h
	gcc -c name_server.c

name_client.o: nameclient.c
	gcc -c nameclient.c
	
client_list.o: client_list.c client_list.h
	gcc -c client_list.c
	
clean:
	rm *.o name_server name_client
