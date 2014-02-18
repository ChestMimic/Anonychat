
#include <openssl/pem.h>

#include "enc.h"


/** Initializes the OpenSSL crypto library for use
*/

void client_initialize_crypto() {
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);
}

/** Cleans up the OpenSSL crypto library after use
*/

void client_cleanup_crypto() {
	EVP_cleanup();
	ERR_free_strings();
}

/** Creates an rsa_ctx_o, initializes it, and returns a pointer to it
	@return A pointer to the newly created rsa_ctx, or NULL if an error occured
*/

rsa_ctx_o* client_create_rsa_ctx() {
	rsa_ctx_o* rsa_ctx = (rsa_ctx_o*) malloc(sizeof(rsa_ctx));
	
	if (rsa_ctx == NULL) {
		//malloc failed.. *tear
		return NULL;
	}
	
	EVP_CIPHER_CTX_init(&(rsa_ctx->rsa_encrypt_ctx));
	EVP_CIPHER_CTX_init(&(rsa_ctx->rsa_decrypt_ctx));
	
	return rsa_ctx;
}

/** Cleans up the specified rsa_ctx
	@param rsa_ctx Pointer to the rsa_ctx_o to clean up
	@return 1 if successful, 0 otherwise
*/

int client_clean_rsa_ctx(rsa_ctx_o* rsa_ctx) {
	free(rsa_ctx);
	return 1;
}

/** Loads a Public key from the file at the path specified
	@param file_path cstring containing the path of the file to load
	@return A pointer to the EVP_PKEY struct representing the public key, or NULL
		if an error occured
*/

EVP_PKEY* client_open_pub_key(char* file_path) {	
	//open the file for reading
	FILE* fp = fopen(file_path, "r");
	if (fp == NULL) {
		//couldnt open file
		return NULL;
	}	
	
	//file is open lets read it into an EVP_PKEY
	EVP_PKEY* pkey = (EVP_PKEY*) malloc(sizeof(EVP_PKEY));
	RSA* rsa_key = PEM_read_RSA_PUBKEY(fp, NULL, 0, NULL);
	
	if (rsa_key == NULL || pkey == NULL) {
		//error creating reading key from file, or malloc failed
		return NULL;
	}
	
	if (!EVP_PKEY_set1_RSA(pkey, rsa_key)) {
		//error setting pkey.
		return NULL;
	}
	
	int res = fclose(fp);
	
	return pkey;	
}


/** Loads a Private key from the file at the path specified
	@param file_path cstring containing the path of the file to load
	@return A pointer to the EVP_PKEY struct representing the private key, or NULL
		if an error occured
*/

EVP_PKEY* client_open_priv_key(char* file_path) {
	//open the file for reading
	FILE* fp = fopen(file_path, "r");
	if (fp == NULL) {
		//couldnt open file
		return NULL;
	}	
	
	//file is open lets read it into an EVP_PKEY
	EVP_PKEY* pkey = (EVP_PKEY*) malloc(sizeof(EVP_PKEY));
	RSA* rsa_key = PEM_read_RSAPrivateKey(fp, NULL, 0, NULL);
	
	if (rsa_key == NULL || pkey == NULL) {
		//error creating reading key from file, or malloc failed
		return NULL;
	}
	
	if (!EVP_PKEY_set1_RSA(pkey, rsa_key)) {
		//error setting pkey.
		return NULL;
	}
	
	int res = fclose(fp);
	
	return pkey;
}

/** Encrypt the given msg with the given public key
	@param msg The message to encrypt
	@param public_key a cstring containing the public key
	@param res A pointer to a message_encrypted_o struct to place the resulting
		encrypted message, iv, and ek
	@return 0 if successful, error code otherwise
*/

int client_encrypt_msg(rsa_ctx_o* rsa_ctx, const unsigned char* msg, EVP_PKEY* public_key, message_encrypted_o* res) {
	int msg_enc_len = 0;
	int block_size = 0;
	int msg_len = 0;
	
	EVP_CIPHER_CTX* encryption_ctx = &(rsa_ctx->rsa_encrypt_ctx);
	
	res->encrypted_key = (unsigned char*) malloc(EVP_PKEY_size(public_key));
	res->init_vector = (unsigned char*) malloc(EVP_MAX_IV_LENGTH);
	
	if (res->encrypted_key == NULL || res->init_vector == NULL) {
		//malloc failed
		return 1;
	}
	
	//set the size of the init vector
	int init_vector_length = EVP_MAX_IV_LENGTH;
	int encrypted_key_len;
	
	res->encrypted_msg = (unsigned char*) malloc(msg_len + EVP_MAX_IV_LENGTH);
	if (res->encrypted_msg == NULL) {
		//malloc failed.
		return 1;
	}
	
	int tmp = EVP_SealInit(encryption_ctx, EVP_aes_256_cbc(), &(res->encrypted_key),
		&encrypted_key_len, res->init_vector, &public_key, 1);
	
	if (!tmp) {
		//evp seal init failed
		return 1;
	}
	
	tmp = EVP_SealUpdate(encryption_ctx, res->encrypted_msg + msg_enc_len, 
		&block_size, msg, msg_len);
		
	if (!tmp) {
		//evp seal update failed
		return 1;
	}
	
	msg_enc_len += block_size;
	
	tmp = EVP_SealFinal(encryption_ctx, res->encrypted_msg + msg_enc_len,
		&block_size);
		
	if (!tmp) {
		//evp seal final failed
		return 1;
	}
	
	msg_enc_len += block_size;
	
	//clean up
	EVP_CIPHER_CTX_cleanup(encryption_ctx);
	
	return 0;	
}

/** Attempts to decypt the given msg
	@param rsa_ctx Pointer to an struct containing the rsa context
	@param msg Pointer a message_encrypted_o which contains the msg, ek, and iv
	@param private_key Pointer to an EVP_PKEY, which contains the private key to decrypt the
		message with
	@return A pointer to the decypted message, or NULL if 
		unable to decrypt
*/

char* client_decrypt_msg(rsa_ctx_o* rsa_ctx, message_encrypted_o* msg, EVP_PKEY* private_key) {
	int decrypt_len = 0;
	int block_size = 0;
	
	int encrypted_msg_len = msg->encrypted_msg_len; // strlen, it while processing the msg
	int encryption_key_len = msg->encrypted_key_len;
	EVP_CIPHER_CTX* decryption_ctx = &(rsa_ctx->rsa_decrypt_ctx);
	
	unsigned char* decrypted_msg = (unsigned char*) malloc(encrypted_msg_len);
	
	int res = EVP_OpenInit(decryption_ctx, EVP_aes_256_cbc(), msg->encrypted_key,
		encryption_key_len, msg->init_vector, private_key);
		
	if (!res) {
		//failed to initialize the decrypt
		return NULL;
	}
	
	res = EVP_OpenUpdate(decryption_ctx, decrypted_msg + decrypt_len, &block_size,
		msg->encrypted_msg, encrypted_msg_len);
	
	if (!res) {
		//failed to update the decrypt
		return NULL;
	}
	decrypt_len += block_size;
	
	res = EVP_OpenFinal(decryption_ctx, decrypted_msg + decrypt_len, &block_size);
	
	if (!res) {
		//failed to finalize the decrypt
		return NULL;
	}
	
	decrypt_len += block_size;
	
	//clean ups
	EVP_CIPHER_CTX_cleanup(decryption_ctx);
	
	return decrypted_msg;
	
}


