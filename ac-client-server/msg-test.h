#ifndef __MSG_TEST_H__
#define __MSG_TEST_H__

typedef struct timeval time_val;

/** Input handle function to be used by client_server, that will send the 
		current time, for a RTT
*/

void* input_handle_rtt(void* arg);

/** Client parse msg function to print out the RTT
*/

int client_parse_msg_rtt(char* msg, int len);

/** Returns the time in milli seconds represented in the given time struct
*/

double get_time_in_milli(time_val* time);

/** Time difference in mill secconds
*/

double get_timediff_milli(time_val* start_time, time_val* end_time);


#endif
