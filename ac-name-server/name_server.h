#ifndef NAME_SERVER_H
#define NAME_SERVER_H


struct _client {
	int socket_fd; // the socket descriptor for this client

};

typedef struct _client client;

/** Prints the usage of this program
*/
void print_usage();

/** Thread function for handling a new client that has connected to the server
	@param arg Pointer to a client struct, represnting the client that has connected
*/

void handle_client(void* arg);

/** Sets up the socket the server will use to listen on for new clients
	@param port cstring that contains the port to start the server on
	@return The descriptor of the socket that was created, or -1 if it failed
*/

int init_server(char* port);



#endif