#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>

#include "msg.h"
#include "list.h"


/*
	GLIB Hash Table Reference 
	https://developer.gnome.org/glib/2.37/glib-Hash-Tables.html
	
	OpenSSL Crypt Library Reference
	https://www.openssl.org/docs/crypto/crypto.html
*/

/** Determines if the string a starts with the string b,
	@param a cstring a
	@param b cstring b
	@return 1 if a starts with b, 0 otherwie
*/

int str_starts_with(const char* a, const char* b) { 
	return strncmp(a, b, strlen(b)) == 0;
}


/** Sends the given message to the specified socket
	@param socket_fd The descriptor of the socket to send the message to
	@param msg A pointer to the message to sent
	@param length The length of the message
	@return 1 if sucessful, 0 otherwise
*/

int send_msg(int socket_fd, void* msg, int length) {
	if (!send(socket_fd, msg, length, 0)) {
		return 0;
	}
	return 1;

}

/** Sends the specified string message to the given peer
	@param peer Pointer to the peer struct to sent the message to
	@param msg The message to send to the peer
	@return 1 if successful, 0 otherwise, -1 if no connection was open
*/
int send_msg_peer(peer_o* peer, char* msg) {
	if (peer->open_con) { // check if the connection is open
		return send_msg(peer->socket_fd, msg, strlen(msg));
	}
	//TODO: change to open a connection to a peer?
	return -1;
}

/** Initializes the OpenSSL crypto library for use
*/

void client_initialize_crypto() {
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);
}

/** Cleans up the OpenSSL crypto library after use
*/

void client_cleanup_crypto() {
	EVP_cleanup();
	ERR_free_strings();
}

/** Attempts to decypt the given msg
	@param msg Pointer a message_encrypted_o which contains the msg, ek, and iv
	@return A pointer to the decypted message, or NULL if 
		unable to decrypt
*/

char* client_decrypt_msg(message_encrypted_o* msg) {

}

/** Encrypt the given msg with the given public key
	@param msg The message to encrypt
	@param public_key a cstring containing the public key
	@param res A pointer to a message_encrypted_o struct to place the resulting
		encrypted message, iv, and ek
	@return 0 if successful, error code otherwise
*/

int client_encrypt_msg(char* msg, EVP_PKEY* public_key, message_encrypted_o* res) {
	size_t msg_enc_len = 0;
	size_t block_size = 0;
	
	res->encrypted_key = (unsigned char*) malloc(EVP_PKEY_size(public_key));
	res->init_vector = (unsigned char*) malloc(EVP_MAX_IV_LENGTH);
	
	if (res->encrypted_key == NULL || res->init_vector == NULL) {
		return 1;
	}
	
	//set the size of the init vector
	size_t init_vector_length = EVP_MAX_IV_LENGTH;
	
	res->encrypted_msg = (unsigned char*) malloc(msg_enc_len + EVP_MAX_IV_LENGTH);
	
	
	
	
}

/** Creates the hash table to be used by the client to store
		messages that have already been processed
	@return A pointer to a GLIB GHashTable
*/

GHashTable* client_create_hash_table() {
	return g_hash_table_new_full(g_str_hash, g_str_equal,
		client_hash_free_key, client_hash_free_val);

}

/** Method used to free the key used by hash table
	@param key Pointer to th key to free
*/

void client_hash_free_key(gpointer key) {
	free(key);
}

/** Function used to free the value used by the hash table
	@param val Pointer to the value to free
*/

void client_hash_free_val(gpointer val) {
	message_hash_o* msg_val = (message_hash_o*) val;
	free(msg_val->msg); // free the cstring containing the msg
	free(msg_val);
}


/** Adds the specified message to the given hash table
	@param hash_table The hashtable to add the message to
	@param msg A cstring containing the message, to add to the table
*/

void client_hash_add_msg(GHashTable* hash_table, char* msg) {
	//allocate and create the struct for the msg val
	message_hash_o* msg_val = (message_hash_o*) malloc (sizeof (message_hash_o));
	msg_val->msg = msg;
	msg_val->purge_time = time(NULL) + MESSAGE_PURGE_TIME;
	
	//insert the value into the hash table
	g_hash_table_insert(hash_table, msg, msg_val);	
}

/** Determines if the client has seen the message before
	@param The client's hash table that contains seen messages
	@param msg The msg to check to see if the client has seen it
	@return 1 if it has been seen before, 0 other wise
*/

int client_has_seen_msg(GHashTable* hash_table, char* msg) {
	if (g_hash_table_contains(hash_table, msg)) {
		return 1; // it is in the hash table, we've seen the msg
	}
	return 0; // not in the hash table, we haven't seen the msg
}

/** Removes all of the expired messages from the specified hash table
	@param hash_table The hash table to purge the emssages from
	@return The number of messages removed from the hash table
*/

unsigned int client_purge_msg(GHashTable* hash_table) {
	return g_hash_table_foreach_remove(hash_table, purge_message, NULL);
}

/** Helper function to purge the hash table. Will remove all messages in which
	the purge_time is less than current_time
	@return TRUE if the message is to be removed, FALSE otherwise
*/

gboolean purge_message(gpointer key, gpointer val, gpointer data) {
	time_t current_time;
	time(&current_time);	
	message_hash_o* msg_val = (message_hash_o*) val;
	
	if (msg_val->purge_time <= current_time) {
		return TRUE;
	}
	return FALSE;
}

