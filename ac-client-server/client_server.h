#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#include <pthread.h>

/** Struct representing a name server
*/

struct _name_server {
	char address[NI_MAXHOST]; // the name servers ip address
	char port[NI_MAXSERV]; // the name severs port
	int socket_fd; // the name servers socket descriptor
	int open_con; // 1 if there is an open connection, 0 if not
	pthread_t* name_thread; // the thread for the name server handler
	
};

typedef struct _name_server name_server_o;

/** Struct representing a client connected as a peer
*/

struct _client {
	int client_id; // the id of this client
	int socket_fd; // the socket descriptor the client is connected on
	int open_con; // 1 if there is an open connection, 0 if not
	pthread_t* handler_thread; // the thread to handle responses from clients
};

typedef struct _client client_o;

/** Prints the appropriate usage of this program
*/
void print_usage();

/** Establishes a connection to the host with the specified
		address and port
	@param address The ip address of the host to  connect to
	@param port The port of the host to connect to
	@return The socket descripto of the connection, or -1 if connection failed.
*/

int connect_to_host(char* address, char* port);

/** Establishes a conenction to the specified name server
	@param name_server A pointer to a name_server_o struct that represents
		the name server to connect to
	@return A socket descriptor of the connection's socket, or -1 if connection 
		failed. Also adds the socket descriptor to the name server struct
*/

int connect_to_name_server(name_server_o* name_server);

/** Informs the name server of the port we are listening on for peer
		connections
	@param name_server A pointer to a struct representing the name server
	@param port A cstring containing the updated port
	@return 0 if sucessful, -1 otherwise
*/

int update_port(name_server_o* name_server, char* port);

/** Function that will handle messages received from the name server
	and take appropriate action
	@param arg A pointer to a struct contaning the argumetns. TODO: define
	@return TODO: define
*/

void* name_server_handle(void* arg);

/** Establishes a connection with the specified peer
	@param peer A pointer to the peer struct to connect to
	@return The socket descriptor of the connection, or -1 if connection failed.
		The socket descriptor will also be stored in the peer struct
*/

int connect_to_peer(peer_o* peer);

/** Initializes the socket to listen for connections on
	@param arg A pointer to the int representing the socket descriptor
	@return 1 if there was an error, 0 if sucessfully exited.
*/

void* listen_for_clients(void* arg);

/** Function that will handle messages received from the specified peer
	@param arg A pointer to a client struct
	@return TODO: define
*/

void* client_handle(void* arg);

/** Function that will handle any command line input that the user enters
	@param arg TODO: define
	@return TODO: define
*/

void* input_handle(void* arg);



#endif
