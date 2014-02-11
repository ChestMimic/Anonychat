#include <string.h>
#include <arpa/inet.h>

#include "msg.h"


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