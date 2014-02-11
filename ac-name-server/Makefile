all: name_server name_client

name_server: name_server.o client_list.o
	gcc name_server.o client_list.o -o name_server -pthread
	
name_client: nameclient.o nameclient_test.o
	gcc nameclient_test.o nameclient.o -o name_client
	
name_server.o: name_server.c name_server.h
	gcc -c name_server.c -pthread

name_client.o: nameclient.c nameclient.h
	gcc -c nameclient.c
	
nameclient_test.o: nameclient_test.c
	gcc -c nameclient_test.c

client_list.o: client_list.c client_list.h
	gcc -c client_list.c -pthread
	
clean:
	rm *.o name_server name_client
