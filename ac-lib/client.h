#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <netdb.h>
#include <sys/types.h>

#include "list.h"


struct _client {
	int socket_fd; // the socket descriptor for this client
	struct sockaddr_storage client_addr; // struct storing the client address
	char address[NI_MAXHOST]; // the string containing the address of the client
	char port[NI_MAXSERV]; // the string containing the port of the client
	pthread_t* client_thread; // pointer to the thread that this client is executing on
	list* connections;
};

typedef struct _client client;

/** Adds b as a connection to a
	@param a the client to add the connection to
	@param b The client to add
*/

void add_connection_client(client* a, client* b);

/** Creates a connection graph between all peers in the given list
	@param client_list The list containing all of the connected clients
*/

void create_connection_graph(list* client_list);

/** Adds all other clients to each client in the specified list
	@param clietn_list The list containing all of the connected clients
*/

void create_connection_all(list* client_list);

/** Makes sure that the given index does not exceded the array size, and if it does
	loops the index around
	@param index THe index to check
	@param size The size of the array
	@return The new index to use
*/

int fix_index(int index, int size);

#endif
