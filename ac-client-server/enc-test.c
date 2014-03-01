
/** Contains the code to run encryption tests

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>

#include "enc.h"
#include "key_table.h"

#define PUBLIC_KEY_DIR "./pub_key/"
#define PRIVATE_KEY_DIR "./priv_key/"

typedef struct timeval time_val;

// the hash table that contains the public keys
GHashTable* public_key_hash_table;

// The rsa struct for encryption
rsa_ctx_o* rsa_encrypt_ctx;

// Alice's private key
EVP_PKEY* private_key_alice;
//Bob's private key
EVP_PKEY* private_key_bob;

void str_rand(char* dest, int len);
double get_timediff_milli(time_val* start_time, time_val* end_time);
EVP_PKEY* load_private_key(char* key_name);
void init_crypto();
void load_public_keys() ;



int main (int argc, char **argv) {
	init_crypto();
	
	//seed the random number gen
	srand(time(NULL));
	
	//the message to encrypt
	
	EVP_PKEY* public_key_alice = key_get_by_name(public_key_hash_table, "alice");
	
	time_val start_time;
	time_val end_time;
	double time;
	
	char* encoded_msg;
	char* decoded_msg;
	
	int i=0;
	for (i=20; i <= 1000; i += 20) {
			
		char* msg = (char*) malloc(i + 1);
		str_rand(msg, i);

		//time the encryption	
		gettimeofday(&start_time, NULL);
		//THIS STUFF IN HERE IS TIMED
		encoded_msg = msg_encrypt_encode(msg, rsa_encrypt_ctx, public_key_alice);		
		gettimeofday(&end_time, NULL);				
		time = get_timediff_milli(&start_time, &end_time);		
		printf("Time to encrypt message with len               %d : %fms \n", i, time);
		
		//time a successful decryption
		gettimeofday(&start_time, NULL);
		//THIS STUFF IN HERE IS TIMED
		decoded_msg = msg_decode_decrypt(encoded_msg,	rsa_encrypt_ctx, private_key_alice);	
		if (decoded_msg == NULL) {
			printf("Decryption failed! \n");
			return 0;
		}
		gettimeofday(&end_time, NULL);				
		time = get_timediff_milli(&start_time, &end_time);		
		printf("Time to sucessfuly dencrypt message with len   %d : %fms \n", i, time);
		
		//time an unsuccessful decryption
		gettimeofday(&start_time, NULL);
		//THIS STUFF IN HERE IS TIMED
		decoded_msg = msg_decode_decrypt(encoded_msg,	rsa_encrypt_ctx, private_key_bob);	
		if (decoded_msg != NULL) {
			printf("Decryption succeded! \n");
			return 0;
		}
		gettimeofday(&end_time, NULL);				
		time = get_timediff_milli(&start_time, &end_time);		
		printf("Time to unsucessfuly dencrypt message with len %d : %fms \n", i, time);
		
		//printf("Encoded msg len %d \n", strlen(encoded_msg));
		
		
		printf("\n");
		//free up some memories
		free(msg);
		free(encoded_msg);
		free(decoded_msg);
		
	}
	
	//char* msg;
	
	//encryption function
	//char* encoded_msg = msg_encrypt_encode(msg, rsa_encrypt_ctx, pub_key);
	
	//decryption function
	//char* decoded_msg = msg_decode_decrypt(msg,	rsa_encrypt_ctx, private_key);
}

void str_rand(char* dest, int len) {
	int i;
	
	for(i = 0; i < len; i++) {
		dest[i] = '0' + rand() % 72; // starting on '0', ending on '}'
	}
	dest[i] = '\0'; // add the null terminator
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

/** Loads the private key with the given name
	@param key_name The name of the private key to load, (with file ext)
*/

EVP_PKEY* load_private_key(char* key_name) {
	printf("Loading Private Key! \n");
	//the private key directory
	char* private_key_dir = PRIVATE_KEY_DIR;
	
	int full_path_len = 256 + 25 + 1;
	char full_path[full_path_len];
	
	strncpy(full_path, private_key_dir, full_path_len);
	strncat(full_path, key_name, full_path_len);
	
	EVP_PKEY* pvt_key = client_open_priv_key(full_path);
	EVP_PKEY* private_key = (EVP_PKEY*) malloc(sizeof(EVP_PKEY));
	memcpy(private_key, pvt_key, sizeof(EVP_PKEY));
	free(pvt_key);
	
	if (private_key == NULL) {
		printf("Unable to open private key %s \n", full_path);
	}
	
	printf("Loaded Private Key! \n");
	return private_key;
	
}

void init_crypto() {
	client_initialize_crypto();
	rsa_encrypt_ctx = client_create_rsa_ctx();

	public_key_hash_table = key_create_hash_table();
		
	//lets load the public keys
	load_public_keys();
	
	//lets load the private key
	private_key_bob = load_private_key("bob.pem");
	private_key_alice = load_private_key("alice.pem");
	
	//load_peer_keys(peer_key_name);
	
}

/** Loads the public keys to be used for encrypting messages

*/

void load_public_keys() {
	printf("Loading Public Key! \n");
	//the public key directory
	char* public_key_dir = PUBLIC_KEY_DIR;
	
	int full_path_len = 256 + 25 + 1;
	char full_path[full_path_len];
	
	//opens the directory
	DIR* dir = opendir(public_key_dir);
	if (dir == NULL) {
		printf("Unable to open public key directory \n");
		return;
	}
	
	struct dirent* file;
	
	while ( (file = readdir(dir)) != NULL) {		
		//we have the full path
		strncpy(full_path, public_key_dir, full_path_len);
		strncat(full_path, file->d_name, full_path_len);		
		
		char* ext = strrchr(file->d_name, '.');
		if (ext == NULL) {
			continue; // no extension here
		}
		
		*ext = '\0';
		ext++;		
		
		if (strncmp(ext, "pub", 4) == 0) {			
		
			EVP_PKEY* key = client_open_pub_key(full_path);
			EVP_PKEY* key_cpy = (EVP_PKEY*) malloc(sizeof(EVP_PKEY));
			memcpy(key_cpy, key, sizeof(EVP_PKEY));
			
			free(key);
			
			if (key == NULL) {
				printf("Unable to open key %s \n", full_path);
			}		
			//add the key to the hash table
			key_hash_add(public_key_hash_table, file->d_name, key_cpy);			

		}		
		memset(full_path, 0, full_path_len);
	}	
	printf("We opened all public keys \n");
}
