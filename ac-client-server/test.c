#include <stdlib.h>
#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h> 


#include       "enc.h"
#include	   "msg.h"
#include       "key_table.h"

#define RSA_KEYLEN 2048



// the hash table that contains the public keys
GHashTable* public_key_hash_table;


EVP_PKEY* private_key;

// The rsa struct for encryption
rsa_ctx_o* rsa_ctx;

void load_public_keys() {

	//the public key directory
	char* public_key_dir = "./pub_key/";
	
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
			if (key == NULL) {
				printf("Unable to open key %s \n", full_path);
			}		
			//add the key to the hash table
			key_hash_add(public_key_hash_table, file->d_name, key);			

		}		
		memset(full_path, 0, full_path_len);
	}	
	printf("We opened all public keys \n");
}

void load_private_key(char* key_name) {

	//the private key directory
	char* private_key_dir = "./priv_key/";
	
	int full_path_len = 256 + 25 + 1;
	char full_path[full_path_len];
	
	strncpy(full_path, private_key_dir, full_path_len);
	strncat(full_path, key_name, full_path_len);
	
	private_key = client_open_priv_key(full_path);
	if (private_key == NULL) {
		printf("Unable to open private key %s \n", full_path);
	}
	
	printf("Loaded Private Key! \n");
	
}

int main (int argc, char **argv) {

	client_initialize_crypto();
	rsa_ctx = client_create_rsa_ctx();

	public_key_hash_table = key_create_hash_table();

	load_public_keys();
	load_private_key("bob.pem");
	
	printf("Starting encryption test! \n");
	
	


	EVP_PKEY* public_key = key_get_by_name(public_key_hash_table, "bob");
	
	//key generation
	/*
	EVP_PKEY* gen_key = (EVP_PKEY*) malloc(sizeof(EVP_PKEY));
	
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
	
	if (!ctx) {
		printf("Error initializing context \n");
	}
	printf("Created Context! \n");
	if (EVP_PKEY_keygen_init(ctx) <=0) {
		printf("Error with keygen init\n");
	}
	printf("Iinit keyGen!\n");
	if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEYLEN) <=0) {
		printf("Error setting keygen bits\n");
	}
	printf("Set keygen bits! \n");
	if (EVP_PKEY_keygen(ctx, &gen_key) <=0 ) {
		printf("Error generating key\n");
	}
	
	EVP_PKEY_CTX_free(ctx);
	
	printf("GenKey %x \n", gen_key);
	
	if (public_key != NULL && private_key != NULL) {
		printf("Opened the public and private key without issue! \n");
	}	*/
	
	//char* msg = "Hello World Mother Fuckers!";
	char* msg = "Can eve help us? I hope so.";
	
	printf("Original Message |%s|\n", msg);
	
	char* encrypted_msg = msg_encrypt_encode(msg, rsa_ctx, public_key);
	
	printf("EncryptedMessage |%s|\n", encrypted_msg);
	
	char* decrypted_msg = msg_decode_decrypt(encrypted_msg, rsa_ctx, private_key);
	
	if (decrypted_msg == NULL) {
		printf("Error occured while decrypting the message \n");
	}
	else {
		printf("DecryptedMessage |%s|\n", decrypted_msg);
	}
	

	return 1;
}