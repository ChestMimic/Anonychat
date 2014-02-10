//Author: Mark Fitzgibbon
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef SERVER_MAX_MESSAGE
#define SERVER_MAX_MESSAGE (512)
#endif

int main(int argc, char* argv[]){
	struct addrinfo hints, *res;
	int sock;

	printf("NameClient? \n");

	memset(&hints, 0, sizeof hints);	//ensure empty struct
	hints.ai_family = AF_INET;		//IPv4 OR IPv6
	hints.ai_socktype = SOCK_STREAM;	//TCP connection
	hints.ai_flags = AI_PASSIVE;

	//TODO:Get nameserver connection info

	getaddrinfo(argv[1], argv[2] , &hints, &res);

	printf("Establishing socket... \n");
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	//get socket number
	if(sock == -1){	//negative socket means error, die
		//error
		printf("Socket failed!\n");
		exit(1);
	}
	printf("Socket good!\n");

	printf("Connecting to server...\n");
	if(connect(sock, res->ai_addr, res->ai_addrlen) == -1){ //attempt connection
		//error
		printf("Connection failed!\n");
		exit(1);
	}
	printf("Connected!\n");

	while(1) {
		char buff[SERVER_MAX_MESSAGE];
		char* mesg = "Hello";
		
		//will this seg fault? lol..
		int resu = recv(sock, buff, SERVER_MAX_MESSAGE, 0); //wait for messages, die on error
		
		if (resu < 0) {
			close(sock);
			freeaddrinfo(res);
			exit(1);
		}
		else{
			printf("%s\n", buff);
		}
		
		int sen = send(sock, mesg, sizeof(mesg), 0);//allows connection to die when nameserver dies
	}

	return 0;
}
