#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <pthread.h>


#include "name_server.h"
#include "client_list.h"


/** list of clients connected to the server */
list* client_list;

pthread_mutex_t priting_mutex; //mutex for updating list

int main(int argc, char* argv[]) {

	//initialize the client list
	client_list = list_create();
	
	//initialize the mutexes
	pthread_mutex_init(&priting_mutex, NULL); 
	
	//initialize the server socket
	int socket_fd = init_server(SERVER_DEF_PORT);
	
	
	//listen for clients
	listen_for_clients(socket_fd);
}

/** Thread function for handling a new client that has connected to the server
	@param arg Pointer to a client struct, represnting the client that has connected
*/

void* handle_client(void* arg) {

	client* client_o = (client*) arg; // cast the argument into client struct

	int client_socket = client_o->socket_fd; // the socket that this client is connected on
	struct sockaddr_storage client_addr = client_o->client_addr; // the address of the client
	
	//TODO: fix this to get IP address, not host-name
	
	struct sockaddr_in* ipv4_addr = (struct sockaddr_in*) &client_addr;
	int ip_addr = ipv4_addr->sin_addr.s_addr;
	
	//copy the ip address into the client struct
	inet_ntop(AF_INET, &ip_addr, client_o->address, sizeof(client_o->address));
	
	/* Old address code
	int res = getnameinfo( (struct sockaddr *) &client_addr, sizeof(struct sockaddr_storage),
		client_o->address, sizeof(client_o->address), NULL, 0, NI_NAMEREQD);
		*/
	//This is working for getting hostname, 68-118-228-238.dhcp.oxfr.ma.charter.com form.
	//However the first connection to the server throws ai_family not supported
		
	pthread_mutex_lock(&priting_mutex); //lock the printing mutex before we print this.
	/*if (res) {
		printf("Error! %s \n", gai_strerror(res));
		pthread_mutex_unlock(&priting_mutex);
		return NULL; //couldn't get the clients hostname, exit the thread
	}
	else {*/
	printf("Connection started from %s!\n", client_o->address);
	//}
	pthread_mutex_unlock(&priting_mutex);

	//handle the client, 
		//add the client to list of connected clients
		//handle any requests from the client.
			//as of now, just requests for peers
		//remove the client from the list when it disconnects
		
	//add the client to the conencted list
	
	printf("Lets add the client to the list \n");
	
	pthread_mutex_lock(&(client_list->mutex)); //obtain list mutex before we operate on it
	list_add(client_list, client_o);
	pthread_mutex_unlock(&(client_list->mutex)); // release mutex when done
		//request handling
	
	printf("Lets remove the client to the list \n");
	
	//remove the client from the connected list	
	pthread_mutex_lock(&(client_list->mutex)); //obtain list mutex before we operate on it
	list_remove(client_list, client_o);
	pthread_mutex_unlock(&(client_list->mutex)); // release mutex when done
	
	printf("Lets free the client \n");
	free(client_o); // no longer need the memory for the client
	
	printf("Lets exit the thread \n");
	
	return NULL;
	
		
}


/** Sets up the socket the server will use to listen on for new clients
	@param port cstring that contains the port to start the server on
	@return The descriptor of the socket that was created, or -1 if it failed
*/

int init_server(char* port) {
	
	struct addrinfo hints; // our address information
	struct addrinfo* server_info;
	struct addrinfo* server_connect;
	
	int socket_fd;
	
	int res = 0; //generic results
	
	//clear out the hints struct
	memset(&hints, 0, sizeof hints);
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // TCP connection
	hints.ai_flags = AI_PASSIVE;
	
	res = getaddrinfo(NULL, port, &hints, &server_info);
	
	if (res) { // error occured
		printf("Error occured while retrieving our address info.\n Unable to start server. \n");
		return -1;
	}
	
	//bind the server to an address
	for (server_connect = server_info; server_connect != NULL;
		server_connect = server_connect->ai_next) {
			
		socket_fd = socket(server_connect->ai_family, server_connect->ai_socktype,
			server_connect->ai_protocol);
		
		if (socket_fd == -1) {
			//cannot bind to this address, try next
		}
		
		int yes = 1; // true int for setsocket opt
		res = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		
		if (res == -1) {
			printf("Could not set socketopt, exiting \n");
			return -1;
		}	
		
		res = bind(socket_fd, server_connect->ai_addr, server_connect->ai_addrlen);
		if (res == -1) {
			close(socket_fd);
			printf("Error binding, trying again \n");
			continue;
		}
		break; // we bound successfuly
	}
	
	//free the memory that was allocated
	free(server_info);
	//free(server_connect); 
	
	return socket_fd;

}

/** Listens for clients 
	@param socket_fd The descriptor of the socket to listen on
	
*/

void listen_for_clients(int socket_fd) {

	int res = listen(socket_fd, SERVER_BACKLOG);
		

	//TODO: Add a way for the server to gracefully exit? Maybe?
	while (1) {
		pthread_mutex_lock(&priting_mutex);
		printf("Waiting for a client to connect \n"); //debug statement mostly
		pthread_mutex_unlock(&priting_mutex);
		
		socklen_t sin_size;
		struct sockaddr_storage client_addr;
		int client_socket_fd; // socket fd for client
		
		client_socket_fd = accept(socket_fd, (struct sockaddr*) &client_addr,  &sin_size);
		
		pthread_mutex_lock(&priting_mutex);
		if (client_socket_fd == -1) {
			printf("Error occured while trying to accept a client...\n");
			pthread_mutex_unlock(&priting_mutex);
			continue; // couldnt accept client, continue on
		}
		
		printf("Client has connected \n");
		pthread_mutex_unlock(&priting_mutex);
		
		client* client_con = (client*) malloc(sizeof(client));
		
		client_con->socket_fd = client_socket_fd;
		client_con->client_addr = client_addr;		
	
		//start the client thread	
		pthread_create(&(client_con->client_thread), NULL, &handle_client, (void*) client_con);
		
	}

}

void print_usage() {
	printf("Usage: \n");
	printf("\t ./name_server");
}
