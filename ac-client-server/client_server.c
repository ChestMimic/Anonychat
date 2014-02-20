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

#define         BUFFER_SIZE     512
#define         CHUNK_SIZE 512

int clientThread(void *);
int inputThread(void *);
void connectToPeer(peer_o* peer, void* data);

// List of peers for a client
list* peer_list;
unsigned int idTracker = 0;
int portNo; // Your port number

int main (int argc, char **argv) {
  
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
  if ( argc < 3 )  {
        printf( "Usage: %s <hostname or IP Address, Port Number>\n", argv[0]);
        exit(0);
  }

  ptr = *++argv;       // Get the argument after the program name
  if ( (hptr = gethostbyname(ptr)) == NULL) {
      printf("gethostbyname error for host: %s: %s\n", ptr, hstrerror(h_errno));
      exit(0);
  }

  // Print Check
  printf("Official Hostname: %s\n", hptr->h_name);

  // Are there other names for this site?
  for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)   {
      printf("    Alias: %s\n", *pptr);
  }

  pptr = hptr->h_addr_list;   // Assumes address type is AF_INET

  for ( ; *pptr != NULL; pptr++) {
      strcpy( IPAddress,  inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
      printf("\taddress: %s\n", IPAddress );
  }

  if ( argc < 3) {
      printf( "The program expects two arguments\n" );
      printf( "Browser <ipaddress> <port number>\n" );
      exit(0);
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
    int out = newThread((void*) (*clientThread), &fd);
    int ou2 = newThread((void*) (*inputThread), &fd);
  }

  char req[100];
  strcpy(req, "PORTUPD");
  send(fd, req, strlen(req), 0);

  sa.sin_family       = AF_INET;
  sa.sin_port         = htons(portNo);     // client & server see same port
  sa.sin_addr.s_addr  = htonl(INADDR_ANY); // the kernel assigns the IP ad

  // Do a bind of that socket
  bind(fd2, (struct sockaddr *) &sa, sizeof(sa));

  listen(fd2, 5);
  int fdListen = fd2;
  int len = sizeof(ca);

  while(0 == 0) {
    // Do the accept
    int fdConn = accept(fdListen, (struct sockaddr*) &ca, &len);
    if(fdConn == -1) {
      printf("Error");
    }
    int out = newThread((void*) (*clientThread), &fdConn);
    int ou2 = newThread((void*) (*inputThread), &fdConn);
  }
}

int clientThread(void* data) {
  
  // variables
  unsigned int cprio, myprioID;
  char inBuff[BUFFER_SIZE];
  char ip_output_buffer[BUFFER_SIZE];

  // Thread things
  sleep(1);
  myprioID = pthread_self();
  int* fdConn = (int*) data;
  cprio = getpriority(PRIO_PROCESS, 0);
  //printf( "New thread: \nID: %ld \nPriority: %d\n", myprioID, cprio);
  //printf("Data passed to the new thread: %d\n", *((int*)data));


  char port[100];
  
  //printf("%s\n", init);

  strncpy(port, "PORT ", sizeof(port));
  char no[100]; 
  sprintf(no, "%d\n", portNo);
  strncat(port, no, strlen(no));

  // Send init string
  send(*(int*)data, port, strlen(port), 0);
  //printf("Sent string: %s\n", init);
  bzero(ip_output_buffer, CHUNK_SIZE);
  
  // Receive 
  while(0 == 0) {
    if (recv( *((int*)data), inBuff, sizeof(inBuff) - 2, 0 ) <= 0 ) {
      printf( "Error encountered: Terminating\n");
      exit(0);
    } else {
      printf( "String received: %s", inBuff);
      // Message containing PEER infomation
      if(strncmp(inBuff, "PEERS ", 6) == 0) {
	char ip[INET_ADDRSTRLEN];
	char* ipf;
	strncpy(ip, inBuff+6, INET_ADDRSTRLEN);
	ipf = strtok(ip, ":");
	printf("%s\n", ip);
	peer_o* peer = (peer_o*)malloc(sizeof(peer));
	peer->peer_id = idTracker;
	strncpy(peer->address, ipf, INET_ADDRSTRLEN);
	peer->socket_fd = 0;
	peer->open_con = 0;
	peer->ttl = 30;
	list_add(peer_list, &peer);
	idTracker++;
	//if(strlen(ipf) >= INET_ADDRSTRLEN - 1) {
	  connectToPeer(peer, data);
	  //}
      }
    }
      bzero(inBuff, CHUNK_SIZE);
    }

  return 0;
}

void connectToPeer(peer_o* peer, void* data) {
 
  struct sockaddr_in pa;
  int lpa = sizeof(pa);
  
  printf("Trying to connect to another peer s1\n");

  // If already connected to peer, return
  int i = 0;
  for( i = 0; i < list_size(peer_list); i++) {
    peer_o* tp = (peer_o*)list_item_at(peer_list, i);
    if(strncmp(tp->address, peer->address, INET_ADDRSTRLEN - 1) == 0) {
      return;
    }
    //if( *(int*)data == peer->socket_fd) {
    // return;
     // }
  }

  if ((peer->socket_fd = socket ( AF_INET, SOCK_STREAM, 0 )) < 0)   {
      printf( "The socket call failed\n");
      exit(1);
  }
  
  pa.sin_family       = AF_INET;
  pa.sin_port         = htons(peer->);     // client & server see same port
  pa.sin_addr.s_addr  = inet_addr(peer->address); // the kernel assigns the IP ad
  
  printf("Trying to connect to another peer s2\n");
  if (connect(peer->socket_fd, (struct sockaddr *)&pa, lpa) == -1)  {
     perror( "Failed to connect");
  } else {
     char init[100];
     peer->open_con = 1;
     int out = newThread((void*) (*clientThread), &peer->socket_fd);
     // Send connect message
     strcpy(init, "Established connection to new client\n");
     send(peer->socket_fd, init, strlen(init), 0);
  }
  
}

int inputThread(void* data) {
  
  // variables
  unsigned int cprio, myprioID;
  char inBuff[BUFFER_SIZE];
  char ip_output_buffer[BUFFER_SIZE];
  int* fdConn = (int*) data;


  // Thread things
  sleep(1);
  myprioID = pthread_self();
  cprio = getpriority(PRIO_PROCESS, 0);
  //printf( "New thread: \nID: %ld \nPriority: %d\n", myprioID, cprio);
  //printf("Data passed to the new thread: %d\n", *((int*)data));
  
  char message[BUFFER_SIZE];

  // Wait while connections establish
  sleep(1);

  // Handle user input
  printf("Hello user. Please enter your message.\n" );
  while(0 == 0) {
    scanf("%s", message);
    int i = 0;
    for( i = 0; i < list_size(peer_list); i++) {
      peer_o* tp = (peer_o*)list_item_at(peer_list, i);
      send(tp->socket_fd, message, strlen(message), 0);
    }
    send(*(int*)data, message, strlen(message), 0);
    printf("Message sent\n");
    bzero(message, CHUNK_SIZE);
  }

  return 0;
}
