#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#define DEFAULT_PEER_PORT "4758"
#define PEER_SERVER_BACKLOG (5)
#define PURGE_FREQUENCY (30) // the purge frequency of messages in seconds

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

/** Struct to represent the peer server
*/

struct _peer_server {
	char port[NI_MAXSERV]; // the port to bind to
	int socket_fd; // the socket the server is bound to
	int open_con; // 1 if there is an open connection, 0 if not
	pthread_t* peer_thread; // the thread that is handling the serv
};

typedef struct _peer_server peer_server_o;

/** Struct representing a client connected as a peer
*/

struct _client {
	int client_id; // the id of this client
	int socket_fd; // the socket descriptor the client is connected on
	int open_con; // 1 if there is an open connection, 0 if not
	struct sockaddr_storage client_addr; // address of the client
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

/** Initializes the socket to listen for connections on
	@param arg A pointer to the peer_server struct
	@return 1 if there was an error, 0 if sucessfully exited.
*/

void* listen_for_clients(void* arg);

/** Sets up the socket the server will use to listen on for new clients
	@param Pointer to a peer_server which contains the port to connect to
	@return The socket descriptor of the socket that was created, -1 if unsucessful
*/

int init_server(peer_server_o* peer_server);


/** Initializes the lib crypto context, the rsa encryption context
		and loads the public / private keys into memory
	@param private_key_name The name of the private key to load
*/

void init_crypto(char* private_key_name);

/** Loads the public keys to be used for encrypting messages

*/

void load_public_keys();

/** Loads the private key with the given name
	@param key_name The name of the private key to load, (with file ext)
*/

void load_private_key(char* key_name);

/** Cleans up the crypto, and frees any unnecesary memory
*/

void cleanup_crypto();

/** Parses and sends the message that the user has typed in
	@param input A cstring contaning the message to parses
	@param len The length of the message
	@return 0 if sucessful, 1 otherwise
*/
int input_send_msg(char* input, int len);

/** Parses the message received from a client. Which means decoding and decrypting 
		the message and takign appropriate action if the message was intended for us
	@param The message received
	@param len The length of the message
	@return 0 if sucessful, 1 otherwise
*/

int client_parse_msg(char* msg, int len);

/** Sends the given message to all of our peers
	@param msg The message to send
	@param len The length of the message to send
	@return 0 if sucessful, 1 otherwise
*/

int client_send_to_all_peers(char* msg, int len);

/** Thread responsible for purging the msg hash table of old messages

	Will call the purge function every PURGE_FREQUENCY seconds
*/

void* client_purge_msg_hash(void* arg);


#endif
