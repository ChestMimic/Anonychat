#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#include <pthread.h>


#include "name_server.h"
#include "client_list.h"


/** list of clients connected to the server */
list* client_list;
started
pthread_mutex_t priting_mutex; //mutex for updating list

int main(int argc, char* argv[]) {

	//initialize the client list
	client_list = list_create();
	
	//initialize the mutexes
	pthread_mutex_init(&priting_mutex, NULL); 
	
	//seed the random generator
	srand(time(NULL));
	
	//initialize the server socket
	int socket_fd = init_server(SERVER_DEF_PORT);
	
	
	//listen for clients
	listen_for_clients(socket_fd);
}

/** Thread function for handling a new client that has connected to the server
	@param arg Pointer to a client struct, represnting the client that has connected
*/

void* client_handle(void* arg) {
	client* client_o = (client*) arg; // cast the argument into client struct
	int client_socket = client_o->socket_fd; // the socket that this client is connected on		
	struct sockaddr_in* ipv4_addr = (struct sockaddr_in*) &(client_o->client_addr); //client addr
	int ip_addr = ipv4_addr->sin_addr.s_addr; //address int form.	
	//copy the ip address into the client struct
	inet_ntop(AF_INET, &ip_addr, client_o->address, sizeof(client_o->address));	
		
	pthread_mutex_lock(&priting_mutex); //lock the printing mutex before we print this.
	printf("Connection started from %s on Socket: %d \n", client_o->address, client_socket);
	pthread_mutex_unlock(&priting_mutex);
	
	//send the initial peer request
	//listen for requests from the client	

	
	pthread_mutex_lock(&(client_list->mutex)); //obtain list mutex before we operate on it
	list_add(client_list, client_o);
	
	client_send_peers(client_o);
	
	pthread_mutex_unlock(&(client_list->mutex)); // release mutex when done
	
	char buffer [SERVER_MAX_MESSAGE + 1];
	int res;
	while ( (res = recv(client_socket, buffer, SERVER_MAX_MESSAGE, 0))) {
		buffer[res + 1] = '\0';
		printf("Received: %s \n", buffer);
		memset(buffer, 0, SERVER_MAX_MESSAGE);
	}
	
	//while (1) { } //busy wait
	
	//remove the client from the connected list	
	pthread_mutex_lock(&(client_list->mutex)); //obtain list mutex before we operate on it
	list_remove(client_list, client_o);
	pthread_mutex_unlock(&(client_list->mutex)); // release mutex when done

	printf("Connection disconnected from %s on Socket: %d \n", client_o->address, client_socket);

	close(client_o->socket_fd);
	free(client_o); // no longer need the memory for the client	
	return NULL;		
}

/** Creates a set of peers for the specified client, and sends them to the client
	@param client_o A pointer to a client structure in which to send the peers to
*/

void client_send_peers(client* client_o) {

	int msg_size = SERVER_MAX_MESSAGE;
	char msg[msg_size]; // the message to use
	msg[0] = '\0'; // add the null byte
	strncat(msg, "PEERS", SERVER_MAX_MESSAGE);
	msg_size = msg_size - 2 - 5; // - 2 for CRLF, -6 for PEERS 
	
	int num_peers = PEER_POOL_SIZE; // the number of peers to send
	int total_peers = list_size(client_list); //total number of connected clients, excludign self
	
	printf("num_peers: %d, total_peers: %d \n", num_peers, total_peers);
	
	if (num_peers > total_peers) {
		//printf("num_peers is > total_peers \n");
		//if num peers is greater than total peers, we can just add the whole list
		num_peers = list_size(client_list) - 1;
		int i;
		for (i = 0; i <total_peers; i++) {
			//printf("Im in da loop? \n");
			client* client_p = (client*)list_item_at(client_list, i);
			if (client_p->socket_fd == client_o->socket_fd) {
				//printf("client_p socket: %d, client_o socket: %d \n", client_p->socket_fd,
					//client_o->socket_fd);
				continue;
			}
			//copy the ip address into the message
			strncat(msg, " ", SERVER_MAX_MESSAGE); //add a space
			strncat(msg, client_p->address, SERVER_MAX_MESSAGE); 
			printf("Concat %s to msg \n", client_p->address);
		}
		
	}
	else {	
		int me = -1;	
		
		//max ip size is 16. probally us INET_ADDRSTRLEN, which is 16
	
		int i;
		for (i = 0; i < num_peers; i++) {
		
			if (msg_size < INET_ADDRSTRLEN) {
				//TODO: add more handling here
				//however with just and address pool of 5, we shouldnt reach max, 5 * 16 = 80 < 510
				break; // we reached message limit.
			}
			int done = 0;
			while (!done) {
				int rnum = rand() % (total_peers); 
				client* client_p = list_item_at(client_list, rnum);
				if (rnum == me || client_p->socket_fd == client_o->socket_fd) {
					me = rnum;
					continue;
				}
				
				//we found a peer that is not us. yay!
				//copy the ip address into the message
				strncat(msg, " ", SERVER_MAX_MESSAGE); //add a space
				strncat(msg, client_p->address, SERVER_MAX_MESSAGE); 
			
			}
		}
	}
	
	strncat(msg, "\r\n", SERVER_MAX_MESSAGE);
	
	printf("Message to send: %s \n", msg);
	
	server_send_message(client_o->socket_fd, msg, strlen(msg));

}

/** Sends the message in msg to the given socket
	@param socket_fd the Socket descriptor of the client to send the message to
	@param Pointer to the message to send
	@param size The size of the message being sent
	@return 0 if sucess, error code other wise */

int server_send_message(int socket_fd, void* msg, int size) {
	if (!send(socket_fd, msg, size, 0)) {
		printf("Error sending message! \n");
		return 1;
	}
	return 0;
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
		
	int count = 0;
	//TODO: Add a way for the server to gracefully exit? Maybe?
	while (1) {
		pthread_mutex_lock(&priting_mutex);
		printf("Waiting for a client to connect \n"); //debug statement mostly
		pthread_mutex_unlock(&priting_mutex);
		
		struct sockaddr_storage client_addr;
		socklen_t sin_size = sizeof(client_addr);
		int client_socket_fd; // socket fd for client
		
		client_socket_fd = accept(socket_fd, (struct sockaddr*) &client_addr,  &sin_size);
		
		pthread_mutex_lock(&priting_mutex);
		if (client_socket_fd == -1) {
			printf("Error occured while trying to accept a client...\n");
			pthread_mutex_unlock(&priting_mutex);
			continue; // couldnt accept client, continue on
		}		
		pthread_mutex_unlock(&priting_mutex);
		
		client* client_con = (client*) malloc(sizeof(client));				
		client_con->socket_fd = client_socket_fd;
		client_con->client_addr = client_addr;	
		client_con->client_thread = (pthread_t*) malloc(sizeof(pthread_t));		
		//start the client thread	
		pthread_create(client_con->client_thread, NULL, &client_handle, (void*) client_con);
		
	}

}

void print_usage() {
	printf("Usage: \n");
	printf("\t ./name_server");
}
