#ifndef NAME_SERVER_H
#define NAME_SERVER_H

#include "client.h"

#define SERVER_BACKLOG (10) // the number of clients that can be queued
#define SERVER_DEF_PORT "6958" // the default port the server will listen on

#define CLIENT_PEER_DEF_PORT "4758" // default client port

#define SERVER_MAX_MESSAGE (1200) // the maximum message size in bytes

#define PEER_POOL_SIZE (5) // the default peer pool size to send to the client


typedef struct timeval time_val;

/** Prints the usage of this program
*/
void print_usage();

/** Thread function for handling a new client that has connected to the server
	@param arg Pointer to a client struct, represnting the client that has connected
*/

void* client_handle(void* arg);

/** Re creates the peer graph, and seends a new peer message to all clients
*/

void client_update_peers();

/** Creates the peer message for the specified client, and then sends the peer
		message to that client
	@param client_o The client to send the message to
*/

void client_send_peers(client* client_o);

/** Sends the message in msg to the given socket
	@param socket_fd the Socket descriptor of the client to send the message to
	@param Pointer to the message to send
	@param size The size of the message being sent
	@return 0 if sucess, error code other wise */

int server_send_message(int socket_fd, void* msg, int size);

/** Determines if the string a starts with the string b,
	@param a cstring a
	@param b cstring b
	@return 1 if a starts with b, 0 otherwise
*/

int str_starts_with(const char* a, const char* b); 

/** Sets up the socket the server will use to listen on for new clients
	@param port cstring that contains the port to start the server on
	@return The descriptor of the socket that was created, or -1 if it failed
*/

int init_server(char* port);

/** Listens for clients 
	@param socket_fd The descriptor of the socket to listen on	
*/

void listen_for_clients(int socket_fd);

/** Time difference in mill secconds
*/

double get_timediff_milli(time_val* start_time, time_val* end_time);


#endif