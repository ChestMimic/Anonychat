#ifndef NAME_SERVER_H
#define NAME_SERVER_H


#define SERVER_BACKLOG (10) // the number of clients that can be queued
#define SERVER_DEF_PORT "6958" // the default port the server will listen on

#define SERVER_MAX_MESSAGE (512) // the maximum message size in bytes

#define PEER_POOL_SIZE (5) // the default peer pool size to send to the client


struct _client {
	int socket_fd; // the socket descriptor for this client
	struct sockaddr_storage client_addr; // struct storing the client address
	char address[INET_ADDRSTRLEN]; // the string containing the address of the client
	pthread_t client_thread; // pointer to the thread that this client is executing on

};

typedef struct _client client;

/** Prints the usage of this program
*/
void print_usage();

/** Thread function for handling a new client that has connected to the server
	@param arg Pointer to a client struct, represnting the client that has connected
*/

void* client_handle(void* arg);

/** Creates a set of peers for the specified client, and sends them to the client
	@param client_o A pointer to a client structure in which to send the peers to
*/

void client_send_peers(client* client_o);

/** Sets up the socket the server will use to listen on for new clients
	@param port cstring that contains the port to start the server on
	@return The descriptor of the socket that was created, or -1 if it failed
*/

int init_server(char* port);

/** Listens for clients 
	@param socket_fd The descriptor of the socket to listen on	
*/

void listen_for_clients(int socket_fd);


#endif