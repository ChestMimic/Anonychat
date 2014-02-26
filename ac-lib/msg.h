#ifndef MSG_LIB_H
#define MSG_LIB_H

#include <glib.h>
#include <netdb.h>
#include <pthread.h>
#include <openssl/evp.h>

#define MESSAGE_PURGE_TIME (30) // the purge time of a message in secconds
#define DEFAULT_PEER_TIMEOUT (30) // the default peer timeout

/** Struct representing a peer on the network
*/

struct _peer {
	int peer_id; // the id of the peer
	char address[NI_MAXHOST]; // the IPv4 address of the peer
	char port[NI_MAXSERV]; // the port the client is listening to peers on
	int socket_fd; // the open socket with the peer
	int open_con; // 1 if there is an open connection with the peer, 0 if not
	int ttl; //the time to live of this peer, in seconds
	pthread_t* handler_thread; // the thread to handle the peer
	EVP_PKEY* public_key; // the public key to communicate with this peer
};

typedef struct _peer peer_o;

/** Struct representing the value to insert into the message
	hash table
*/

struct _message_hash {
	char* msg; // the msg
	time_t purge_time; // of this time is in the past, purge the hash
};

typedef struct _message_hash message_hash_o;

//TODO: create a function to clean up this msg, need to free all 3 char pointers
struct _message_encrypted {
	unsigned char* encrypted_msg; // The message encrypted with the encrypted key
	unsigned char* encrypted_key; //The encrypted key, which is encrypted with the RSA pub key
	unsigned char* init_vector; // The IV used during encryption
	int encrypted_msg_len; // the length of encrypted_msg
	int encrypted_key_len; // the length of the encrypted key
	int init_vector_len; // the length of the init vector
};

typedef struct _message_encrypted message_encrypted_o; 
 

/** Creates a new peer with the given ip address and default time to live
	@param ip_addr The ip address of the peer
	@param port The port for the peer
	@return A pointer to a peer_o struct
*/	

peer_o* create_peer(char* ip_addr, char* port);

/** Determines if the string a starts with the string b,
	@param a cstring a
	@param b cstring b
	@return 1 if a starts with b, 0 otherwise
*/

int str_starts_with(const char* a, const char* b);

/** Sends the given message to the specified socket
	@param socket_fd The descriptor of the socket to send the message to
	@param msg A pointer to the message to sent
	@param length The length of the message
	@return 1 if sucessful, 0 otherwise
*/

int send_msg(int socket_fd, void* msg, int length);

/** Sends the specified string message to the given peer
	@param peer Pointer to the peer struct to sent the message to
	@param msg The message to send to the peer
	@param len  The length of the message to send
	@return 1 if successful, 0 otherwise
*/
int send_msg_peer(peer_o* peer, char* msg, int len);

/** Creates the hash table to be used by the client to store
		messages that have already been processed
	@return A pointer to a GLIB GHashTable
*/

GHashTable* client_create_hash_table();

/** Function used to free the key used by hash table
	@param key Pointer to th key to free
*/

void client_hash_free_key(gpointer key);

/** Function used to free the value used by the hash table
	@param val Pointer to the value to free
*/

void client_hash_free_val(gpointer val);

/** Adds the specified message to the given hash table
	@param hash_table The hashtable to add the message to
	@param msg A cstring containing the message, to add to the table
*/

void client_hash_add_msg(GHashTable* hash_table, char* msg);

/** Determines if the client has seen the message before
	@param The client's hash table that contains seen messages
	@param msg The msg to check to see if the client has seen it
	@return 1 if it has been seen before, 0 other wise
*/

int client_has_seen_msg(GHashTable* hash_table, char* msg);

/** Removes all of the expired messages from the specified hash table
	@param hash_table The hash table to purge the emssages from
	@return The number of messages removed from the hash table
*/

unsigned int client_purge_msg(GHashTable* hash_table);

/** Helper function to purge the hash table. Will remove all messages in which
	the purge_time is less than current_time
	@return TRUE if the message is to be removed, FALSE otherwise
*/

gboolean purge_message(gpointer key, gpointer val, gpointer data);

#endif
