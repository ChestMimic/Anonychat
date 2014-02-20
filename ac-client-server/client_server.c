#include       <stdlib.h>
#include       <sys/socket.h>
#include       <netinet/in.h>
#include       <arpa/inet.h>
#include       <stdio.h>
#include       <string.h>
#include       <netdb.h>
#include       <stdlib.h>
#include       <time.h>
#include       <pthread.h>
#include       <sys/resource.h>


#include       "thread_util.h"
#include       "msg.h"
#include       "list.h"
#include	   "client_server.h"

#define         BUFFER_SIZE     512
#define         CHUNK_SIZE 512

//int clientThread(void *);
//int inputThread(void *);
//void connectToPeer(peer_o* peer, void* data);

int idTracker;

// List of peers for a client
list* peer_list;

//indicates whether or not we are still running
int running = 1; 

int portNo; // Your port number

pthread_mutex_t priting_mutex; // mutex for printing

void print_usage() {
	printf("Usage: \n");
	printf("\t	client-server name-server-addr name-server-port peer-port \n");
	printf("\t ex: client-server 192.168.1.105 6958 4758 \n");
}

int main (int argc, char **argv) {
  
  char* address_name_server; // the address of the name server
  char* port_name_server; // the port of the name server
  char* port_peers; // the port to listen for peer connections on
  
  if (argc != 4) {	// If there are not 4 arguments, error
	print_usage();
	return 1;
  }
  
  address_name_server = argv[1]; // name server address is 2nd argument
  port_name_server = argv[2]; // name server port is 3rd argument
  port_peers = argv[3]; // peer port is 4th argument
  
  // All variables and such
  int fd, fd2;
  struct sockaddr_in sa, ca;
  int lsa = sizeof(sa);
  int lca = sizeof(ca);
  char *ptr, **pptr;
  char str[INET6_ADDRSTRLEN];
  struct hostent     *hptr;
  char IPAddress[100];
  char ip_input_buffer[BUFFER_SIZE]; // Received buffer
  char ip_output_buffer[BUFFER_SIZE]; // Sending buffer

  // Input arguements
  if ( argc != 3 )  {
        printf( "Usage: %s <hostname or IP Address, Port Number>\n", argv[0]);
        exit(0);
  }

  //ptr = *++argv;       // Get the argument after the program name
  if ( (hptr = gethostbyname(argv[1])) == NULL) {
      printf("gethostbyname error for host: %s: %s\n", ptr, hstrerror(h_errno));
      exit(0);
  }

  pptr = hptr->h_addr_list;   // Assumes address type is AF_INET

  for ( ; *pptr != NULL; pptr++) {
      strcpy( IPAddress,  inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
      printf("\taddress: %s\n", IPAddress );
  }



  // Initialize list of peers
  peer_list = list_create();
  
  // Set port number
  portNo = atoi(argv[1]);
  printf("Your port: %d\n", portNo);

  if ((fd = socket ( AF_INET, SOCK_STREAM, 0 )) < 0)   {
      printf( "The socket call failed\n");
      exit(1);
  }

  if ((fd2 = socket ( AF_INET, SOCK_STREAM, 0 )) < 0)   {
      printf( "The socket call failed\n");
      exit(1);
  }

  ca.sin_family       = AF_INET;
  ca.sin_port         = htons(6958);     // client & server see same port
  ca.sin_addr.s_addr  = inet_addr(IPAddress); // the kernel assigns the IP ad

  if (connect(fd, (struct sockaddr *)&ca, lca) == -1)  {
     perror( "Failed to connect");
  } else {
   // int out = newThread((void*) (*clientThread), &fd);
    //int ou2 = newThread((void*) (*inputThread), &fd);
  }

  char req[100];
  strcpy(req, "PORTUPD ");
  char no[100]; 
  sprintf(no, "%d\n", portNo);
  strncat(req, no, strlen(no));
  send(fd, req, strlen(req), 0);

  sa.sin_family       = AF_INET;
  sa.sin_port         = htons(portNo);     // client & server see same port
  sa.sin_addr.s_addr  = htonl(INADDR_ANY); // the kernel assigns the IP ad

  // Do a bind of that socket
  bind(fd2, (struct sockaddr *) &sa, sizeof(sa));

  listen(fd2, 5);
  int fdListen = fd2;
  int len = sizeof(sa);

  while(0 == 0) {
    // Do the accept
    int fdConn = accept(fdListen, (struct sockaddr*) &sa, &len);
    printf("Connection\n");
    if(fdConn == -1) {
      printf("Error");
    }
    //int out = newThread((void*) (*clientThread), &fdConn);
    //int ou2 = newThread((void*) (*inputThread), &fdConn);

  }
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
		printf("Recevied |%s| from name server! \n");
	}
	
	printf("Disconnected from name server \n");
	
	name_server->socket_fd = -1;
	name_server->open_con = 0;
	
	return;
	
}

/** Function that will handle messages received from the specified peer
	@param arg A pointer to a peer struct
	@return TODO: define
*/

void* peer_handle(void* arg) {
	// we will handle a peer.
	//basicly just listen for messages from others for now? yea?
	
	peer_o* peer = (peer_o*) arg;
	
	char* buffer = (char*) malloc(BUFFER_SIZE + 1);
	int res;
	
	while ( (res = recv(peer->socket_fd, buffer, BUFFER_SIZE, 0))) {
		buffer[res] = '\0'; //add a null terminator just in case		
		printf("Received: |%s| from peer: %d \n", buffer, peer->peer_id);		
	}
	
	printf("Disconnected from peer: %d \n", peer->peer_id);
	
	// peer has disconnected, open connection is false.
	peer->socket_fd = -1;
	peer->open_con = 0;
	
	return;
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
	
	strncpy(msg, "PORTUPD ", strlen("PORTUPD "));
	strncat(msg, port, NI_MAXSERV);
	
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
	
	//start the thread to handle the peer	
	peer->peer_thread = (pthread_t*) malloc(sizeof(pthread_t));	
	pthread_create(peer->peer_thread, NULL, &peer_handle, (void*) peer);
	
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

void* input_handle(void* arg) {
	//allocate space for the input buffer
	char* input_buffer = (char*) malloc(BUFFER_SIZE);
	size_t buffer_len = BUFFER_SIZE -1; // allow room for the \0
	
	//listen for user input while we are running
	while (running) {
		int len = getline(&input_buffer, &buffer_len, stdin);		
		input_buffer[len] = '\0'; // add the null terminator just in case	
			
		//lock the peer list mutex
		pthread_mutex_lock(&(peer_list->mutex));		
		//for now send any message the user enters to each peer
		int i;
		for (i=0;i<list_size(peer_list); i++) {
			peer_o* to_send = list_item_at(peer_list, i);
			int res = send_msg_peer(to_send, input_buffer);
			if (res == -1) {
				//connection to the peer was not open :(
				//lets NOT open a connection while we have the
				//	list mutex though
			}
		}		
		pthread_mutex_unlock(&(peer_list->mutex)); //unlock mutex
	}
	
	free(input_buffer); // free the allocated memory
	
	return 0;	
}

