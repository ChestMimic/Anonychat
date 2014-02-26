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

unsigned int newThread(void *tsa, int *data) {
    pthread_t            thread;
    pthread_attr_t       attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
    pthread_create( &thread, &attr, tsa, (void *)data );
    pthread_attr_destroy( &attr );

    return( (unsigned int)thread );
}
