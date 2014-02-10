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

#define         BUFFER_SIZE     512
#define         CHUNK_SIZE 512

int clientThread(void *);
unsigned int newThread(void*, int*);

main (int argc, char **argv) {
  
  // All variables and such
  int portNo; // Your port number
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
  if ( argc < 1 )  {
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

  // set up the sockaddr_in structure.  This uses, among other things the 
  // port you've just entered.
  ca.sin_family       = AF_INET;
  ca.sin_port         = htons(portNo);     // client & server see same port
  // Use 127.0.0.1 for local host
  //printf("Your port: %d\n", sa.sin_port );
  ca.sin_addr.s_addr  = inet_addr(IPAddress); // the kernel assigns the IP ad

  if (connect(fd, (struct sockaddr *)&ca, lca) == -1)  {
      perror( "Failed to connect");
  }

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
    printf("accept\n");
    int out = newThread((void*) (*clientThread), &fdConn);
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
  printf( "New thread: \nID: %ld \nPriority: %d\n", myprioID, cprio);
  printf("Data passed to the new thread: %d\n", *((int*)data));

  char init[100];
  strcpy(init, "Established connection to new client");

  // Send init string
  send(*(int*)data, init, strlen(init), 0);
  //printf("Sent string: %s\n", init);
  bzero(ip_output_buffer, CHUNK_SIZE);
  
  // Receive 
  while(0 == 0) {
    /*if (recv( *((int*)data), inBuff, sizeof(inBuff) - 2, 0 ) <= 0 ) {
      printf( "Error encountered: Terminating\n");
      exit(0);
    } else {
      printf( "String received: %s\n", inBuff);
    }
    bzero(inBuff, CHUNK_SIZE);*/
  }

  return(0);
}

unsigned int newThread(void *tsa, int *data) {
    pthread_t            thread;
    pthread_attr_t       attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
    pthread_create( &thread, &attr, tsa, (void *)data );
    pthread_attr_destroy( &attr );

    return( (unsigned int)thread );
}
