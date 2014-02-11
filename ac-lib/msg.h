#ifndef MSG_LIB_H
#define MSG_LIB_H

#define ENCRYPTION_HEADER "AC_ENC_FW"

/** Struct representing a peer on the network
*/

struct _peer {
	int peer_id; // the id of the peer
	char address[INET_ADDRSTRLEN]; // the IPv4 address of the peer
	int socket_fd; // the open socket with the peer
	int open_con; // 1 if there is an open connection with the peer, 0 if not
	int ttl; //the time to live of this peer, in seconds
	//TODO: Possibly add a thread pointer here
};

typedef struct _peer peer_o;


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
	@return 1 if successful, 0 otherwise
*/
int send_msg_peer(peer_o* peer, char* msg);


/** Parses a message that is received by a client
	@param msg The message to parse
	@return 1
*/

int client_parse_msg(char* msg);

/** Attempts to decypt the given msg
	@param msg The message to attempt to decrypt
	@return A pointer to the decypted message, or NULL if 
		unable to decrypt
*/

char* client_decrypt_msg(char* msg);

/** Encrypt the given msg with the given public key
	@param msg The message to encrypt
	@param public_key a cstring containing the public key
	@return A pointer to the encrypted message, or NULL if
		unable to decrpt
*/

char* client_encrypt_msg(char* msg, char* public_key);


#endif