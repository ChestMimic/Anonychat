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
#include <unistd.h>

#include "list.h"
#include "enc.h"
#include "msg-test.h"
#include "client_server.h"


#define NUM_NODES 18

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

extern char* node_name;

// the number of messages processed
int messages_processed = 0;

/** Input handle function to be used by client_server, that will send the 
		current time, for a RTT
*/

void* input_handle_rtt(void* arg) {
	//allocate space for the input buffer

	
	printf("Send a time message every 10 secconds \n");	
	
	char* time_msg = (char*) malloc(70);
	
	while (running) {
		time_val time;
		gettimeofday(&time, NULL);		
		
		int node_num = rand() % NUM_NODES + 1; // 1 -2
		
		double time_milli = get_time_in_milli(&time);
		snprintf(time_msg, 70, "node%d:%s %f",node_num, node_name, time_milli);	
		//TODO: Prolly add our node name, aka node1
		input_send_msg(time_msg, strlen(time_msg));
		sleep(10); // sleep for 10 seconds
	}
	
	free(time_msg); // free the allocated memory
	
	return 0;	
}

/** Input handle function to be used by client_server, that will be used for
		the network utilization test to send random messages at a random interval
*/

void* input_handle_util(void* arg) {
	//allocate space for the input buffer
	

	
	printf("Send a time message every 10 secconds \n");	
	
	char* msg = (char*) malloc(200);
	
	while (running) {
		
		int random_len = rand() % 100 + 20; // random number from 20 - 120
		int node_num = rand() % NUM_NODES + 1; // 1 -2
		
		char* random = (char*) malloc(random_len + 1);
		str_rand(random, random_len);
		
		snprintf(msg, random_len + 1, "node%d:%s", node_num, random);	
		//TODO: Prolly add our node name, aka node1
		input_send_msg(msg, strlen(msg));
		
		free(random);
		int sleep_time = rand() % 30 + 6;
		sleep(sleep_time); // sleep for 10 seconds
	}
	
	free(msg); // free the allocated memory
	
	return 0;	
}

/** Input handle function to be used by client_server, that will be used for
		the scalability test.
		
		Have every clietn send a random message every 10 seccons
*/

void* input_handle_scale(void* arg) {
	//allocate space for the input buffer
	
	printf("Send a time message every 10 secconds \n");	
	
	char* time_msg = (char*) malloc(70);
	
	while (running) {
		time_val time;
		gettimeofday(&time, NULL);		
		
		int node_num = rand() % NUM_NODES + 1; // 1 -2
		
		double time_milli = get_time_in_milli(&time);
		snprintf(time_msg, 70, "node%d:%s %f", node_num, node_name, time_milli);	
		//TODO: Prolly add our node name, aka node1
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
	//printf("Added TO HASH %s \n", msg);
	
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
		
		char* node_from = strtok(decoded_msg, " ");
		char* str_start_time = strtok(NULL, " ");
			
		double time_milli = get_time_in_milli(&time);
		double start_time = atof(str_start_time);
		
		double rtt = time_milli - start_time;
		
		printf("RTT from %s to %s : %f ms\n", node_from, node_name, rtt);

	}
	else {
		//printf("This message was not meant for me :( \n");
	}
}

/** Client parse msg function to print out the RTT
*/

int client_parse_msg_scale(char* msg, int len) {
	time_val start_time;
	time_val end_time;
	gettimeofday(&start_time, NULL); // starting time	

	
	messages_processed++;
	//decrypt the message first.

	
	pthread_mutex_lock(&mht_mutex);
	if (client_has_seen_msg(message_hash_table, msg)) {
		//printf("We have seen this message before! \n");
		pthread_mutex_unlock(&mht_mutex);
		
		gettimeofday(&end_time, NULL); // starting time
	
		double msg_proc_time = get_timediff_milli(&start_time, &end_time);
	
		printf("%s has processed message #%d, took %f ms \n", node_name, messages_processed,
			msg_proc_time);
			
		
		
		return 0;
	}

	
	//add the message to the hash table
	client_hash_add_msg(message_hash_table, msg);
	
	pthread_mutex_unlock(&mht_mutex);
	
	//printf("Added TO HASH %s \n", msg);
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
		
		char* node_from = strtok(decoded_msg, " ");
		char* str_start_time = strtok(NULL, " ");
			
		double time_milli = get_time_in_milli(&time);
		double start_time = atof(str_start_time);
		
		double rtt = time_milli - start_time;
		
		printf("RTT from %s to %s : %f ms\n", node_from, node_name, rtt);

	}
	else {
		//printf("This message was not meant for me :( \n");
	}
	
	gettimeofday(&end_time, NULL); // starting time
	
	double msg_proc_time = get_timediff_milli(&start_time, &end_time);
	
	printf("%s has processed message #%d, took %f ms \n", node_name, messages_processed,
		msg_proc_time);
	
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
