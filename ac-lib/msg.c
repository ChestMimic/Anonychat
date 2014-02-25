#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/pem.h>

#include "msg.h"
#include "list.h"


int peer_id = 0;

/*
	GLIB Hash Table Reference 
	https://developer.gnome.org/glib/2.37/glib-Hash-Tables.html
	
	OpenSSL Crypt Library Reference
	https://www.openssl.org/docs/crypto/crypto.html
*/


/** Creates a new peer with the given ip address and default time to live
	@param ip_addr The ip address of the peer
	@param port The port for the peer
	@return A pointer to a peer_o struct
*/	

peer_o* create_peer(char* ip_addr, char* port) {
	peer_o* peer = (peer_o*) malloc(sizeof(peer_o));
	peer-> peer_id = peer_id++;
	strncpy(peer->address, ip_addr, NI_MAXHOST);
	strncpy(peer->port, port, NI_MAXSERV);
	peer->ttl = DEFAULT_PEER_TIMEOUT;
	peer->open_con = 0;
	return peer;
}

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
	@param len  The length of the message to send
	@return 1 if successful, 0 otherwise, -1 if no connection was open
*/
int send_msg_peer(peer_o* peer, char* msg, int len) {
	if (peer->open_con) { // check if the connection is open
		return send_msg(peer->socket_fd, msg, len);
	}
	//TODO: change to open a connection to a peer?
	return -1;
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
	if (g_hash_table_lookup(hash_table, msg)) {
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

