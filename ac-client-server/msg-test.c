/**
	Contains code for testing related to messages being sent
	
	Most importantly a thread function that can be called to send
		messages containing a time, and functions that keep track of
		the processing time of messages and so on.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include "list.h"
#include "enc.h"
#include "msg-test.h"
#include "client_server.h"

extern int running;

// the hash table that contains the public keys
extern GHashTable* public_key_hash_table;

// the hash table that contains the messages
extern GHashTable* message_hash_table;

// The private key we will use to decrypt messages
extern EVP_PKEY* private_key;

extern pthread_mutex_t pkht_mutex; // public key hash table mutex

extern pthread_mutex_t mht_mutex; // message hash table mutex

// The rsa struct for encryption
extern rsa_ctx_o* rsa_encrypt_ctx;

/** Input handle function to be used by client_server, that will send the 
		current time, for a RTT
*/

void* input_handle_rtt(void* arg) {
	//allocate space for the input buffer
	
	int node_num = 1;
	
	printf("Send a time message every 10 secconds \n");	
	
	char* time_msg = (char*) malloc(50);
	
	while (running) {
		time_val time;
		gettimeofday(&time, NULL);		
		
		double time_milli = get_time_in_milli(&time);
		snprintf(time_msg, 50, "node%d:%f", node_num, time_milli);	
			
		input_send_msg(time_msg, strlen(time_msg));
		sleep(10); // sleep for 10 seconds
	}
	
	free(time_msg); // free the allocated memory
	
	return 0;	
}

/** Client parse msg function to print out the RTT
*/

int client_parse_msg_rtt(char* msg, int len) {
	//decrypt the message first.

	
	pthread_mutex_lock(&mht_mutex);
	if (client_has_seen_msg(message_hash_table, msg)) {
		//printf("We have seen this message before! \n");
		pthread_mutex_unlock(&mht_mutex);
		return 0;
	}

	
	//add the message to the hash table
	client_hash_add_msg(message_hash_table, msg);
	
	pthread_mutex_unlock(&mht_mutex);
	//try to decode the message
	
	//send the message before we attempt to decrypt the message
	//this should hide any possible timing differences between a sucessful or failed
	//encryption.
	client_send_to_all_peers(msg, len); // send orig, not decoded

	//msg_decode_decrypt probally changes msg... 
	char* decoded_msg = msg_decode_decrypt(msg,	rsa_encrypt_ctx, private_key);
	
	if (decoded_msg != NULL) {
		time_val time;
		gettimeofday(&time, NULL);		
		double time_milli = get_time_in_milli(&time);
		double start_time = atof(decoded_msg);
		
		double rtt = time_milli - start_time;
		
		printf("StartTime: %s RTT: %f \n", decoded_msg, rtt);

	}
	else {
		//printf("This message was not meant for me :( \n");
	}
}

/** Returns the time in milli seconds represented in the given time struct
*/

double get_time_in_milli(time_val* time) {
    long seconds = time->tv_sec;
    long micros = time->tv_usec;
    double time_in_seconds = seconds + ((double)micros * pow(10,-6));
    double time_in_milli = time_in_seconds * pow(10,3);	
    return time_in_milli;
}

/** Time difference in mill secconds
*/

double get_timediff_milli(time_val* start_time, time_val* end_time) {
    long second_diff = end_time->tv_sec - start_time->tv_sec;
    long micro_diff = end_time->tv_usec - start_time->tv_usec;
    double time_in_seconds = second_diff + ((double)micro_diff * pow(10,-6));
    double time_in_milli = time_in_seconds * pow(10,3);	
    return time_in_milli;
}
