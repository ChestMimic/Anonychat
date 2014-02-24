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
#include "list.h"
#include "tree.h"


/** list of clients connected to the server */
list* client_list;

pthread_mutex_t priting_mutex; 

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

/** Determines if the string a starts with the string b,
	@param a cstring a
	@param b cstring b
	@return 1 if a starts with b, 0 otherwie
*/

int str_starts_with(const char* a, const char* b) { 
	return strncmp(a, b, strlen(b)) == 0;
}

/** Thread function for handling a new client that has connected to the server
	@param arg Pointer to a client struct, represnting the client that has connected
*/

void* client_handle(void* arg) {
	client* client_o = (client*) arg; // cast the argument into client struct
	int client_socket = client_o->socket_fd; // the socket that this client is connected on		
	struct sockaddr_in* ipv4_addr = (struct sockaddr_in*) &(client_o->client_addr); //client addr	
	ipv4_addr->sin_family = AF_INET;
	
	int res = getnameinfo((struct sockaddr*) ipv4_addr, sizeof(struct sockaddr_in), client_o->address, 
		NI_MAXHOST,	NULL, 0, NI_NUMERICHOST);
		
	if (res) {
		printf("Error getting client info: %s\n", gai_strerror(res));
	}
		
	strncpy(client_o->port, CLIENT_PEER_DEF_PORT, NI_MAXSERV); //copy in the default port
		
	pthread_mutex_lock(&priting_mutex); //lock the printing mutex before we print this.
	printf("Connection started from %s:%s on Socket: %d \n", client_o->address, client_o->port,
		client_socket);
	pthread_mutex_unlock(&priting_mutex);
	
	//send the initial peer request
	//listen for requests from the client	

	
	pthread_mutex_lock(&(client_list->mutex)); //obtain list mutex before we operate on it
	list_add(client_list, client_o);
	
	client_send_peers(client_o);
	
	
	pthread_mutex_unlock(&(client_list->mutex)); // release mutex when done
	
	char buffer [SERVER_MAX_MESSAGE + 1];
	
	while ( (res = recv(client_socket, buffer, SERVER_MAX_MESSAGE, 0))) {
		buffer[res] = '\0';
		printf("Received: %s \n", buffer);
		
		
		if (str_starts_with(buffer, "PEERREQ")) {
			//peer request message
			client_send_peers(client_o); // client requested peers, send them
		}
		else if (str_starts_with(buffer, "PORTUPD")) {
			char* tok = strtok(buffer, " ");
			tok = strtok(NULL, " ");
			if (tok == NULL) {
				printf("Client sent us an invalid PORTUPD, ignoring \n");
				continue; // continue onward!
			}
			
			strncpy(client_o->port, tok, NI_MAXSERV);
			printf("Client %s on socket %d updated thier port to %s \n", client_o->address, 
				client_socket, client_o->port);
				
			//probally not the best place for this,,
			//TODO: Move / figure out where to put this
			//lets send the peers to all other clients
			if (list_size(client_list) <= PEER_POOL_SIZE) {
				int i = 0;
				for (i = 0; i < list_size(client_list); i++) {
					client* to_send = (client*) list_item_at(client_list, i);
					if (to_send->socket_fd == client_socket) {
						continue;
					}
					client_send_peers(to_send);
				}
			}
			
		}
		
		memset(buffer,0, SERVER_MAX_MESSAGE + 1);
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

void client_send_peers(client* client_o, node* graph) {

	int msg_size = SERVER_MAX_MESSAGE;
	char msg[msg_size]; // the message to use
	msg[0] = '\0'; // add the null byte
	strncat(msg, "PEERS ", SERVER_MAX_MESSAGE);
	msg_size = msg_size - 2 - 6; // - 2 for CRLF, -6 for PEERS 
	
	int num_peers = PEER_POOL_SIZE; // the number of peers to send
	int total_peers = list_size(client_list); //total number of connected clients, excludign self
	
	printf("num_peers: %d, total_peers: %d \n", num_peers, total_peers);
	
	if (num_peers > total_peers) {
		char* to_cat = client_peers_static(client_o, msg_size);		
		if (to_cat == NULL) {
			return; // no peers to send
		}
		strncat(msg, to_cat, SERVER_MAX_MESSAGE);
		free(to_cat);
	}
	else {	
		char* to_cat = client_peers_rand(client_o, msg_size);
		strncat(msg, to_cat, SERVER_MAX_MESSAGE);
		free(to_cat);
	}
	
	printf("Message to send: %s \n", msg);
	
	server_send_message(client_o->socket_fd, msg, strlen(msg));

}

/** Crafts the peer message to send to the client when the number of peers is above the peer threshold
	@param client_o The client who is requesting the peers
	@param max_msg_size  The maximum size of the message
	@return A pointer to a cstring containing the list of peers, seperated by a space,
		should be freed after use, or NULL if there are no peers to send
*/

char* client_peers_rand(client* client_o, int max_msg_size) {
	int me = -1;	
	int num_peers = PEER_POOL_SIZE; // the number of peers to send
	int total_peers = list_size(client_list); //total number of connected clients, excludign self
	
	char* msg = (char*) malloc(max_msg_size); //allocate space for the mesage
	memset(msg, 0, max_msg_size);
	msg[0] = '\0';
		
	//max ip size is 16. probally us INET_ADDRSTRLEN, which is 16

	int i;
	for (i = 0; i < num_peers; i++) {	
		while (1) {
			int rnum = rand() % (total_peers); 
			client* client_p = list_item_at(client_list, rnum);
			if (rnum == me || client_p->socket_fd == client_o->socket_fd) {
				me = rnum;
				continue;
			}
			//we found a peer that is now us,
			strncat(msg, client_p->address, max_msg_size);
			strncat(msg, ":", max_msg_size);
			strncat(msg, client_p->port, max_msg_size); 
			strncat(msg, " ", max_msg_size); //add a space
			break;		
		}
	}
	
	if (strlen(msg) > 0) {
		msg[strlen(msg) -1] = '\0'; // remove the extra space at the end
	}
	
	return msg;
}

/** Returns the peer message to send to the client when the number of peers
	is below the peer threshold
	@param client_o The client who is requesting the peers
	@param max_msg_size  The maximum size of the message
	@return A pointer to a cstring containing the list of peers, seperated by a space,
		should be freed after use
*/

char* client_peers_static(client* client_o, int max_msg_size) {
	char* msg = (char*) malloc(max_msg_size);
	memset(msg, 0, max_msg_size);
	msg[0] = '\0';
	//if num peers is greater than total peers, we can just add the whole list
	int total_peers = list_size(client_list);
	
	if (total_peers == 1) {
		return NULL;
	}
	
	int i;
	for (i = 0; i <total_peers; i++) {
		client* client_p = (client*)list_item_at(client_list, i);
		if (client_p->socket_fd == client_o->socket_fd) {
			continue;
		}
		//copy the ip address into the message
		strncat(msg, client_p->address, max_msg_size);
		strncat(msg, ":", max_msg_size);
		strncat(msg, client_p->port, max_msg_size); 
		strncat(msg, " ", max_msg_size); //add a space
	}
	if (strlen(msg) > 0) {
		msg[strlen(msg) -1] = '\0'; // remove the extra space at the end
		printf("Removing extra space at end \n");
	}
	return msg;
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
			printf("Error occured while trying to accept a client. Port is occupied. Is there another program using port 6958?\n");
			pthread_mutex_unlock(&priting_mutex);
			exit(1);
			//continue; // couldnt accept client, continue on
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


void manage_graph(){
	time_t t;
	srand((unsigned) time(&t));

	pthread_mutex_lock(&(client_list->mutex));
	//Part 1: create randomly arranged array
	node* randArray = malloc(sizeof(node)*(client_list->size));
	//convert linked list to array of nodes
	int counter = 0;
	list_elm* here = client_list->head;
	while(counter < client_list->size){
		randArray[counter] = *(node *) createNode(here->val);
		here = here->next;
		counter++;
	}

	//use Fisher-Yates modern shuffle (http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm) to shuffle array
	for(counter = (client_list->size) - 1; counter > 0; counter--){
		int num = rand() % counter;
		//exchange
		node temp = randArray[num];
		randArray[num] = randArray[counter];
		randArray[counter] = temp;
	}
	
	//Part 2: Create graph from array (using tree's functions)
	node* graph = combineNodesToGraph(randArray, 4, client_list->size);
	//Part 3: Send out results to nodes
	//for every client
	here = client_list->head;
	while(here != NULL){
		client_send_peers(here->val , graph);
		here = here->next;
	}
	//release mutex
	pthread_mutex_unlock(&(client_list->mutex));
}
