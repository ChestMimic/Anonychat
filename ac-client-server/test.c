#include <stdlib.h>
#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>

#include       "enc.h"
#include	   "msg.h"
#include       "key_table.h"

#define RSA_KEYLEN 2048

// the hash table that contains the public keys
GHashTable* public_key_hash_table;


EVP_PKEY* public_key;
EVP_PKEY* private_key;

// The rsa struct for encryption
rsa_ctx_o* rsa_ctx;

int main (int argc, char **argv) {

	printf("Starting encryption test! \n");
	
	client_initialize_crypto();
	rsa_ctx = client_create_rsa_ctx();
	
	//create the message hash table
	public_key_hash_table = key_create_hash_table();
	
	char* public_key_path = "./pub_key/bob.pub";
	char* private_key_path = "./priv_key/bob.pem";

	public_key = client_open_pub_key(public_key_path);
	private_key = client_open_priv_key(private_key_path);
	
	//key generation
	
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
	
	printf("Created key! \n");
	EVP_PKEY_CTX_free(ctx);
	
	printf("GenKey %x \n", gen_key);
	
	if (public_key != NULL && private_key != NULL) {
		printf("Opened the public and private key without issue! \n");
	}	
	
	char* msg = "Hello World Mother Fuckers!";
	
	//printf("Original Message |%s|\n", msg);
	
	char* encrypted_msg = msg_encrypt_encode(msg, rsa_ctx, gen_key);
	
	//printf("EncryptedMessage |%s|\n", encrypted_msg);
	
	char* decrypted_msg = msg_decode_decrypt(encrypted_msg, rsa_ctx, gen_key);
	
	if (decrypted_msg == NULL) {
		printf("Error occured while decrypting the message \n");
	}
	else {
		printf("DecryptedMessage |%s|\n", decrypted_msg);
	}
	

	return 1;
}