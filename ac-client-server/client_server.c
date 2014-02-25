#include       <stdlib.h>
#include       <sys/socket.h>
#include       <stdio.h>
#include       <string.h>
#include       <netdb.h>
#include       <stdlib.h>
#include       <pthread.h>
#include       <errno.h>

#include       "msg.h"
#include       "list.h"
#include       "client_server.h"
#include       "enc.h"
#include       "key_table.h"

#define         BUFFER_SIZE     512


// List of peers for a client
list* peer_list;

// list of all connected clients
list* client_list;

// the hash table that contains the public keys
GHashTable* public_key_hash_table;

// the hash table that contains the messages
GHashTable* message_hash_table;

// The private key we will use to decrypt messages
EVP_PKEY* private_key;

//indicates whether or not we are still running
int running = 1; 

//counter for client ids
int client_id = 0;

// The rsa struct for encryption
rsa_ctx_o* rsa_encrypt_ctx;

pthread_mutex_t printing_mutex; // mutex for printing

pthread_mutex_t pkht_mutex; // public key hash table mutex

pthread_mutex_t mht_mutex; // message hash table mutex

void print_usage() {
	printf("Usage: \n");
	printf("\t	client-server name-server-addr name-server-port peer-port private_key \n");
	printf("\t ex: client-server 192.168.1.105 6958 4758 bob.pem\n");
}

/** Initializes the lib crypto context, the rsa encryption context
		and loads the public / private keys into memory
	@param private_key_name The name of the private key to load
*/

void init_crypto(char* private_key_name) {
	client_initialize_crypto();
	rsa_encrypt_ctx = client_create_rsa_ctx();

	public_key_hash_table = key_create_hash_table();
		
	//lets load the public keys
	load_public_keys();
	
	//lets load the private key
	load_private_key(private_key_name);
	
}

/** Loads the private key with the given name
	@param key_name The name of the private key to load, (with file ext)
*/

void load_private_key(char* key_name) {
	printf("Loading Private Key! \n");
	//the private key directory
	char* private_key_dir = "./priv_key/";
	
	int full_path_len = 256 + 25 + 1;
	char full_path[full_path_len];
	
	strncpy(full_path, private_key_dir, full_path_len);
	strncat(full_path, key_name, full_path_len);
	
	EVP_PKEY* pvt_key = client_open_priv_key(full_path);
	private_key = (EVP_PKEY*) malloc(sizeof(EVP_PKEY));
	memcpy(private_key, pvt_key, sizeof(EVP_PKEY));
	free(pvt_key);
	
	if (private_key == NULL) {
		printf("Unable to open private key %s \n", full_path);
	}
	
	printf("Loaded Private Key! \n");
	
}

/** Loads the public keys to be used for encrypting messages

*/

void load_public_keys() {
	printf("Loading Public Key! \n");
	//the public key directory
	char* public_key_dir = "./pub_key/";
	
	int full_path_len = 256 + 25 + 1;
	char full_path[full_path_len];
	
	//opens the directory
	DIR* dir = opendir(public_key_dir);
	if (dir == NULL) {
		printf("Unable to open public key directory \n");
		return;
	}
	
	struct dirent* file;
	
	while ( (file = readdir(dir)) != NULL) {		
		//we have the full path
		strncpy(full_path, public_key_dir, full_path_len);
		strncat(full_path, file->d_name, full_path_len);		
		
		char* ext = strrchr(file->d_name, '.');
		if (ext == NULL) {
			continue; // no extension here
		}
		
		*ext = '\0';
		ext++;		
		
		if (strncmp(ext, "pub", 4) == 0) {			
		
			EVP_PKEY* key = client_open_pub_key(full_path);
			EVP_PKEY* key_cpy = (EVP_PKEY*) malloc(sizeof(EVP_PKEY));
			memcpy(key_cpy, key, sizeof(EVP_PKEY));
			
			free(key);
			
			if (key == NULL) {
				printf("Unable to open key %s \n", full_path);
			}		
			//add the key to the hash table
			key_hash_add(public_key_hash_table, file->d_name, key_cpy);			

		}		
		memset(full_path, 0, full_path_len);
	}	
	printf("We opened all public keys \n");
}

/** Cleans up the crypto, and frees any unnecesary memory
*/

void cleanup_crypto() {
	client_cleanup_crypto();
	free(rsa_encrypt_ctx);
}

int main (int argc, char **argv) {
  
	char* address_name_server; // the address of the name server
	char* port_name_server; // the port of the name server
	char* port_peers; // the port to listen for peer connections on
	char* private_key_name;

	if (argc < 5) {	// If there are not 4 arguments, error
		print_usage();
		return 1;
	}

	address_name_server = argv[1]; // name server address is 2nd argument
	port_name_server = argv[2]; // name server port is 3rd argument
	port_peers = argv[3]; // peer port is the 4th argument
	private_key_name = argv[4]; // the private key is 5th argument


	//create the message hash table
	message_hash_table = client_create_hash_table();
	
	//initialize mutexes
	pthread_mutex_init(&printing_mutex, NULL);
	pthread_mutex_init(&pkht_mutex, NULL);
	pthread_mutex_init(&mht_mutex, NULL);
	
	//initialize peer_list
	peer_list = list_create();
	client_list = list_create();

	// Initialize the crypto + public keys
	init_crypto(private_key_name);

	
	name_server_o name_server;
	strncpy(name_server.address, address_name_server, NI_MAXHOST);
	strncpy(name_server.port, port_name_server, NI_MAXSERV);
	
	//lets setup the server before we conenct to name server
	peer_server_o peer_server;
	strncpy(peer_server.port, port_peers, NI_MAXSERV);
	printf("Peer Server Port: %s\n", peer_server.port);
	peer_server.peer_thread = (pthread_t*) malloc(sizeof(pthread_t));
	
	int server_fd = init_server(&peer_server);
	
	pthread_create(peer_server.peer_thread, NULL, &listen_for_clients, 
		(void*) &peer_server);	

	
	//connect to the name server
	int res = connect_to_name_server(&name_server);
	
	if (res == -1) {
		printf("Unable to connect to name server \n");
		return 0;
	}
	//inform the name server of the port to use.
	res = update_port(&name_server, peer_server.port);  	
	
	//start user input now
	pthread_t user_input_thread;
	pthread_create(&user_input_thread, NULL, &input_handle, NULL);
	
	//start message purging thread
	pthread_t msg_purge_thread;
	pthread_create(&msg_purge_thread, NULL, &client_purge_msg_hash, NULL);
	
	//wait for the name server handler thread to finish.
	//if it does, we will drop out, can't do much without the name server
	pthread_join(*(name_server.name_thread), NULL);
	running = 0;
	
	printf("We are exiting \n");
}

/** Function that will handle messages received from the name server
	and take appropriate action
	@param arg A pointer to a struct contaning the argumetns. TODO: define
	@return TODO: define
*/

void* name_server_handle(void* arg) {
	name_server_o* name_server = (name_server_o*) arg;
	
	char* buffer = (char*) malloc(BUFFER_SIZE + 1);
	int res;
	
	while ( (res = recv(name_server->socket_fd, buffer, BUFFER_SIZE, 0))) {
		buffer[res] = '\0'; // add a null terminator just in case
		printf("Recevied |%s| from name server! \n", buffer);
		
		//parse the peers message
		if (str_starts_with(buffer, "PEERS ")) {
			parse_peers(peer_list, buffer + 6);
			//we parsed the peers now lets connect
			int failed = connect_to_peers(peer_list);
			if (failed) {
				printf("Failed to connect to %d peers \n", failed);
			}
		}
	}
	
	printf("Disconnected from name server \n");
	
	name_server->socket_fd = -1;
	name_server->open_con = 0;
	
	return;
	
}

/** Function that will handle messages received from the specified client
	@param arg A pointer to a client struct
	@return TODO: define
*/

void* client_handle(void* arg) {
	// we will handle a connected client
	//basicly just listen for messages from others for now? yea?

	client_o* client = (client_o*) arg;
	
	char* buffer = (char*) malloc(BUFFER_SIZE + 1);
	int res;
	
	while ( (res = recv(client->socket_fd, buffer, BUFFER_SIZE, 0))) {
		buffer[res] = '\0'; //add a null terminator just in case		
		printf("Received: |%s| from client: %d \n", buffer, client->client_id);		
		
		client_parse_msg(buffer, res);
	}
	
	printf("Disconnected from client: %d \n", client->client_id);
	
	// peer has disconnected, open connection is false.
	close(client->socket_fd); // close the socket descriptor.
	client->socket_fd = -1;
	client->open_con = 0;
	
	//remove the client from the client list
	pthread_mutex_lock(&(client_list->mutex));
	list_remove(client_list, client);
	pthread_mutex_unlock(&(client_list->mutex));
	
	//TODO: Add something to free the client, this will probally error.
	free(client->handler_thread);
	free(client);		

	return;
}

/** Function that will parse the peers from the peer message from name server, into
		the peer list
	@param peer_list A pointer to a list struct containing the current peers,
		without the "PEERS "
	@param peer_msg A cstring containing the peer message from the name server
	@return 0 if sucessful, 1 otherwise
*/

int parse_peers(list* peer_list, char* peer_msg) {	
	if (strlen(peer_msg) < 1) {
		return 1; // no peers to parse
	}
	//clean out the existing peers
	clean_peers_list(peer_list);
			
	char* save_ptr;
	char* ip_port_tok = strtok_r(peer_msg, " ", &save_ptr);	
	
	while (ip_port_tok != NULL) {
		
		
		char* ip_addr = strtok(ip_port_tok, ":");
		char* port = strtok(NULL, ":");
		
		if (ip_addr == NULL || port == NULL) {
			//error parsing this port.
			printf("Error parsing %s \n", ip_port_tok);
			continue;
		}		
		
		ip_port_tok = strtok(NULL, " ");
		peer_o* peer = create_peer(ip_addr, port);
		printf("Created peer %s:%s, with id %d \n", ip_addr, port, peer->peer_id);
		
		pthread_mutex_lock(&(peer_list->mutex));
		list_add(peer_list, (void*) peer);
		pthread_mutex_unlock(&(peer_list->mutex));
		ip_port_tok = strtok_r(NULL, " ", &save_ptr);
	}	
	
	return 0;
}

/** Connects to every peer in the specified peer list
	@param peer_list The list of peers to connect to
	@retun 0 if sucessful, otherwise number of peers that failed to connect
*/

int connect_to_peers(list* peer_list) {
	int failed = 0;
	
	int i=0;
	for (i=0;i < list_size(peer_list); i++) {
		peer_o* to_con = list_item_at(peer_list, i);
		if (!to_con->open_con) {
			//only connect if we are not alraedy connected
			int res = connect_to_peer(to_con);
			if (res == -1) {
				failed ++;
				//we failed to connect to a peer
			}
		}
	}
	
	return failed;
}

/** Removes all elements from the peer list, and frees up thier memory
	@param peer_list A pointer to a list struct that contains the list of peers
	@return 0 if sucessful, 1 otherwise
*/

int clean_peers_list(list* peer_list) {
	pthread_mutex_lock(&(peer_list->mutex));
	while (list_size(peer_list) > 0) {
		peer_o* tmp_peer = (peer_o*) list_item_at(peer_list, 0);
		list_remove(peer_list, tmp_peer);
		if (tmp_peer->open_con) {
			close(tmp_peer->socket_fd);
		}
		free(tmp_peer);
	}
	pthread_mutex_unlock(&(peer_list->mutex));
	return 0;
}

/** Establishes a conenction to the specified name server
	@param name_server A pointer to a name_server_o struct that represents
		the name server to connect to
	@return A socket descriptor of the connection's socket, or -1 if connection 
		failed. Also adds the socket descriptor to the name server struct
*/

int connect_to_name_server(name_server_o* name_server) {
	int socket_fd = connect_to_host(name_server->address, 
		name_server->port);
	
	if (socket_fd == -1) {
		printf("We were unable to connect to the name server! \n");
		return -1;
	}
	
	name_server->socket_fd = socket_fd; // add the socket fd
	name_server->open_con = 1; // set the open connection flag
	
	//start the thread to handle the name server
	name_server->name_thread = (pthread_t*) malloc(sizeof(pthread_t));	
	pthread_create(name_server->name_thread, NULL, &name_server_handle,
		 (void*) name_server);
	
	return socket_fd;

}

/** Informs the name server of the port we are listening on for peer
		connections
	@param name_server A pointer to a struct representing the name server
	@param port A cstring containing the updated port
	@return 1 if sucessful, 0 otherwise
*/

int update_port(name_server_o* name_server, char* port) {
	int msg_len = strlen("PORTUPD ") + NI_MAXSERV + 1;
	char* msg = (char*) malloc(msg_len);
		
	strncpy(msg, "PORTUPD ", msg_len);
	strncat(msg, port, msg_len);

	return send_msg(name_server->socket_fd, msg, msg_len);
}

/** Establishes a connection with the specified peer
	@param peer A pointer to the peer struct to connect to
	@return The socket descriptor of the connection, or -1 if connection failed.
		The socket descriptor will also be stored in the peer struct
*/

int connect_to_peer(peer_o* peer) {
	int socket_fd = connect_to_host(peer->address, peer->port);
	
	if (socket_fd == -1) {
		printf("Could not connect to peer! \n");
	}
	
	peer->socket_fd = socket_fd;
	peer->open_con = 1;	
	return socket_fd;
	
}


/** Initializes the socket to listen for connections on
	@param arg A pointer to the int representing the socket descriptor
	@return 1 if there was an error, 0 if sucessfully exited.
*/

void* listen_for_clients(void* arg) {
	peer_server_o* peer_server = (peer_server_o*) arg;
	int socket_fd = peer_server->socket_fd;

	int res = listen(socket_fd, PEER_SERVER_BACKLOG);
	
	while (running) {
		
		struct sockaddr_storage client_addr;
		socklen_t sin_size = sizeof(client_addr);
		
		
		int client_socket_fd = accept(socket_fd, (struct sockaddr*) &client_addr,
			&sin_size);
			
		if (client_socket_fd == -1) {
			printf("An error occured while trying to accept a client... \n");
			continue;
		}
		
		client_o* client = (client_o*) malloc(sizeof(client_o));
		client->client_id = client_id++;
		client->socket_fd = client_socket_fd;
		client->open_con = 1;
		client->client_addr = client_addr;
		client->handler_thread = (pthread_t*) malloc(sizeof(pthread_t));
		
		
		//start the client thread
		pthread_create(client->handler_thread, NULL, &client_handle, (void*) client);
		
	}
	
}

/** Sets up the socket the server will use to listen on for new clients
	@param port A cstring that contains the port to listen on
	@return The socket descriptor of the socket that was created, -1 if unsucessful
*/

int init_server(peer_server_o* peer_server) {
	struct addrinfo hints;
	struct addrinfo* server_info;
	struct addrinfo* server_connect;
	
	int socket_fd = -1;
	
	int res = 0; //generic results code
	
	memset(&hints, 0, sizeof(hints));
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // tcp
	hints.ai_flags = AI_PASSIVE;
	
	res = getaddrinfo(NULL, peer_server->port, &hints, &server_info);
	
	if (res) { //couldnt get address info
		printf("Error occured while retreiving our addess info. Couldnt start server \n");
		return -1;
	}
	
	for (server_connect = server_info; server_connect != NULL;
		server_connect = server_connect->ai_next) {
			
		socket_fd = socket(server_connect->ai_family, server_connect->ai_socktype,
			server_connect->ai_protocol);
			
			
		if (socket_fd == -1) {
			//cannot bind to this address try again
			printf("Couldn't bind to this addres \n");
			continue;
		}
		
		int yes = 1;
		res = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		
		if (res == -1) {
			close(socket_fd);
			socket_fd = -1;
			printf("Socket opt failed \n");
			continue;
		}
		
		res = bind(socket_fd, server_connect->ai_addr, server_connect->ai_addrlen);
		
		if (res == -1) {
			close(socket_fd);
			socket_fd = -1;
			printf("Bind failed err: %s\n", strerror(errno));
			
			continue;
		}
		
		break; // we bound successfuly
	}
	
	//free(server_info); //dont think this needs to be freed.
	
	printf("Inited server with a socket of %d \n", socket_fd);
	
	peer_server->socket_fd = socket_fd;
	
	return socket_fd;
	
}

/** Establishes a connection to the host with the specified
		address and port
	@param address The ip address of the host to  connect to
	@param port The port of the host to connect to
	@return The socket descripto of the connection, or -1 if connection failed.
*/

int connect_to_host(char* address, char* port) {
	struct addrinfo hints;
	struct addrinfo* host_info;
	
	memset(&hints, 0, sizeof(hints));
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;
	
	int res = getaddrinfo(address, port, &hints, &host_info);
	
	if (res) {
		goto error;
	}
	
	int socket_fd = socket(host_info->ai_family, host_info->ai_socktype,
		host_info->ai_protocol); // open a socket
	
	if (socket_fd == -1) {
		goto error;
	}
	
	res = connect(socket_fd, host_info->ai_addr, host_info->ai_addrlen);
	if (res == -1) {
		goto error;
	}
	
	return socket_fd;
	
	//error case
	error:	
		printf("An error occured while trying to connect to host: %s:%s \n",
			address, port);
		return -1;


}

/** Function that will handle any command line input that the user enters
	@param arg TODO: define
	@return TODO: define
*/

/** Parsing messages yay 
	Format:
		|username:msg body|
		
		Username will be the name of the user to send the message to
		msg body will be the body of the message
		Message body can contain spaces, however username cannot.
		Username and Message body are space delim
		aka, The first characters before the space are used as the username
*/



void* input_handle(void* arg) {
	//allocate space for the input buffer
	char* input_buffer = (char*) malloc(BUFFER_SIZE);
	size_t buffer_len = BUFFER_SIZE -1; // allow room for the \0
	
	//let user know its time
	printf("You are free to type messages! Format username (SPACE) message body \n");
	//listen for user input while we are running
	while (running) {
		int len = getline(&input_buffer, &buffer_len, stdin);	
		if (len > 0) {
			len --;
		}	
		input_buffer[len] = '\0'; //overwrite the \n with \0
		input_send_msg(input_buffer, len);
	
	}
	
	free(input_buffer); // free the allocated memory
	
	return 0;	
}

/** Parses and sends the message that the user has typed in
	@param input A cstring contaning the message to parses
	@param len The length of the message
	@return 0 if sucessful, 1 otherwise
*/


int input_send_msg(char* input, int len) {
	char* first_colon = strchr(input, ':');
	(*first_colon) = '\0'; // set it to terminator char
	char* name = input;
	char* msg = first_colon + 1;
	
	printf("Send Message |%s|  to |%s| \n", msg, name);
	EVP_PKEY* pub_key = key_get_by_name(public_key_hash_table, name);
	if (pub_key == NULL) {
		printf("Unable to send message, No public key for %s exists.\n", name);
		return 1;
	}
	//now we will encrypt the message
	printf("About to encrypt + encode! \n");
	printf("MSG:|%s| CTX: |%x| KEY: |%x|\n", msg, rsa_encrypt_ctx, pub_key);
	char* encoded_msg = msg_encrypt_encode(msg, rsa_encrypt_ctx, pub_key);
	if (encoded_msg == NULL) {
		printf("Error encoding the message \n"); //dont just silently fail..
		return 1;
	}
	
	printf("Encoded message to send! %s \n");
	
	//send to all of our peers!
	client_send_to_all_peers(msg); 
	
	free(encoded_msg); // free the message
	return 0; // we sent all the messages
}


/** Parses the message received from a client. Which means decoding and decrypting 
		the message and takign appropriate action if the message was intended for us
	@param The message received
	@param len The length of the message
	@return 0 if sucessful, 1 otherwise
*/


int client_parse_msg(char* msg, int len) {
	
	pthread_mutex_lock(&mht_mutex);
	if (client_has_seen_msg(message_hash_table, msg)) {
		//we saw the emsasge before, do nothing
		pthread_mutex_unlock(&mht_mutex);
		return 0;
	}
	
	//we havent seen the message, lets process it
	
	//add the message to the hash table
	client_hash_add_msg(message_hash_table, msg);
	
	pthread_mutex_unlock(&mht_mutex);
	
	//try to decode the message
	char* decoded_msg = msg_decode_decrypt(msg, rsa_encrypt_ctx, private_key);
	
	if (decoded_msg != NULL) {
		//woohoo we decoded the message
		printf("Received: %s\n", decoded_msg);
	}
	
	//send to all of our peers!
	client_send_to_all_peers(msg); // send orig, not decoded
}


/** Sends the given message to all of our peers
	@param msg The message to send
	@return 0 if sucessful, 1 otherwise
*/

int client_send_to_all_peers(char* msg) {
	//now lets send the message to all our peers
	pthread_mutex_lock(&(peer_list->mutex));		
	int i;
	for (i=0;i<list_size(peer_list); i++) {
		peer_o* to_send = list_item_at(peer_list, i);
		
		printf("Sending message |%s| to peer %d \n", msg, to_send->peer_id);
					
		int res = send_msg_peer(to_send, msg);
		if (res == -1) {
			//connection to the peer was not open :(
			//TODO: figure out what to do here
		}
	}		
	pthread_mutex_unlock(&(peer_list->mutex)); //unlock mutex
}

/** Thread responsible for purging the msg hash table of old messages

	Will call the purge function every PURGE_FREQUENCY seconds
*/

void* client_purge_msg_hash(void* arg) {

	while (running) {

		sleep( PURGE_FREQUENCY); // sleep for purge frequency seconds

		pthread_mutex_lock(&mht_mutex);
		//clean up the messages	
		client_purge_msg(message_hash_table);
	
		pthread_mutex_unlock(&mht_mutex);
	
	}

}

