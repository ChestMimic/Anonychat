#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <string.h>
#include <stdlib.h>

#include "enc.h"

void print_hex(unsigned char* bytes, int len) {
	int i=0;
	for (i=0;i<len;i++) {
		printf("%02X", bytes[i]);
	}
	printf("\n");
}

void print_msg_struct(message_encrypted_o* msg) {
	printf("EM: ");
	print_hex(msg->encrypted_msg, msg->encrypted_msg_len);
	
	printf("EK: ");
	print_hex(msg->encrypted_key, msg->encrypted_key_len);
	
	printf("IV: ");
	print_hex(msg->init_vector, msg->init_vector_len);
	/*
	printf("EM |%s|\n", msg->encrypted_msg);
	printf("EK |%s|\n", msg->encrypted_key);
	printf("IV |%s|\n", msg->init_vector);*/
	
	
	printf("EML %d\n", msg->encrypted_msg_len);
	printf("EKL %d\n", msg->encrypted_key_len);
	printf("IVL %d\n", msg->init_vector_len);
}

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
	rsa_ctx_o* rsa_ctx = (rsa_ctx_o*) malloc(sizeof(rsa_ctx_o));
	
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
	
	//printf("Opening file %s \n", file_path);
	
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

int client_encrypt_msg(rsa_ctx_o* rsa_ctx, const unsigned char* msg, 
	EVP_PKEY* public_key, message_encrypted_o* res) {
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
		return 2;
	}
	
	tmp = EVP_SealUpdate(encryption_ctx, res->encrypted_msg + msg_enc_len, 
		&block_size, msg, msg_len);
		
	if (!tmp) {
		//evp seal update failed
		return 3;
	}
	
	msg_enc_len += block_size;
	
	tmp = EVP_SealFinal(encryption_ctx, res->encrypted_msg + msg_enc_len,
		&block_size);
		
	if (!tmp) {
		//evp seal final failed
		return 4;
	}
	
	msg_enc_len += block_size;
	
	//update the length fields in the encoded message struct
	res->encrypted_msg_len = msg_enc_len;
	res->encrypted_key_len = encrypted_key_len;
	res->init_vector_len = init_vector_length;
	
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

char* client_decrypt_msg(rsa_ctx_o* rsa_ctx, message_encrypted_o* msg, 
	EVP_PKEY* private_key) {
	
	int decrypt_len = 0;
	int block_size = 0;
	
	int encrypted_msg_len = msg->encrypted_msg_len; // strlen, it while processing the msg
	int encryption_key_len = msg->encrypted_key_len;
	int init_vector_len = msg->init_vector_len;
	
	EVP_CIPHER_CTX* decryption_ctx = &(rsa_ctx->rsa_decrypt_ctx);
	
	unsigned char* decrypted_msg = (unsigned char*) malloc(encrypted_msg_len 
		+ init_vector_len + 1);
		
	memset(decrypted_msg, 0, encrypted_msg_len + init_vector_len + 1);
	printf("DecryptedMsg: |%s|\n", (char*)decrypted_msg);
	
	int res = EVP_OpenInit(decryption_ctx, EVP_aes_256_cbc(), msg->encrypted_key,
		encryption_key_len, msg->init_vector, private_key);
	
	
	if (!res) {
		//failed to initialize the decrypt
		printf("Failed to initialize the decrypt \n");
		return NULL;
	}
	
	printf("DecryptedMsg: |%s|\n", (char*)decrypted_msg);
	
	res = EVP_OpenUpdate(decryption_ctx, decrypted_msg + decrypt_len, &block_size,
		msg->encrypted_msg, encrypted_msg_len);
	
	if (!res) {
		//failed to update the decrypt
		printf("Failed to update the decrypt \n");
		return NULL;
	}
	
	printf("DecryptedMsg: |%s|\n", (char*)decrypted_msg);
	decrypt_len += block_size;
	
	res = EVP_OpenFinal(decryption_ctx, decrypted_msg + decrypt_len, &block_size);
	
	if (!res) {
		//failed to finalize the decrypt
		printf("Failed to finalize the decrypt \n");
		printf("Decrypted msg: |%s| \n", decrypted_msg);
		return NULL;
	}
	
	decrypt_len += block_size;
	
	//clean ups
	EVP_CIPHER_CTX_cleanup(decryption_ctx);
	
	printf("DecryptedMsg: |%s|\n", (char*)decrypted_msg);
	printf("Decrypted_len: %d BlockSize: %d\n", decrypt_len, block_size);
	
	return decrypted_msg;
	
}

/** Parses an encrytped message struct into a string
	@param encrypted_msg A pointer to the encrypted_msg_o containing the encrypted message
	@param dest A pointer to a cstring to store the string in, free after use
	@return The lenght of the string
*/

int parse_encrypted_msg_str(message_encrypted_o* encrypted_msg, char** dest) {
	//determine the length of the parsed message
	int encoded_len = Base64encode_len(encrypted_msg->encrypted_msg_len);
	encoded_len += Base64encode_len(encrypted_msg->encrypted_key_len);
	encoded_len += Base64encode_len(encrypted_msg->init_vector_len);
	encoded_len += 3; // add length of the two spaces + null terminator
	
	(*dest) = (char*) malloc(encoded_len);
	char* tmp = (char*) malloc(encoded_len);
	
	int len = Base64encode(tmp, encrypted_msg->encrypted_msg, 
		encrypted_msg->encrypted_msg_len + 1);
	strncpy((*dest), tmp, len);
	strncat((*dest), " ", 2); //add the space delim
	
	//printf("TMP! |%s|\n", tmp);
	
	memset(tmp, 0, encoded_len); //zero out the memory
	
	len = Base64encode(tmp, encrypted_msg->encrypted_key, encrypted_msg->encrypted_key_len) + 1;
	strncat((*dest), tmp, len);
	strncat((*dest), " ", 2);
	
	//printf("TMP! |%s|\n", tmp);
	memset(tmp, 0, encoded_len); //zero out the memory
	
	len = Base64encode(tmp, encrypted_msg->init_vector, encrypted_msg->init_vector_len) + 1;
	strncat((*dest), tmp, len);
	
	//printf("TMP! |%s|\n", tmp);
	
	free(tmp); // free the memory used by the encoding
	
	return encoded_len;	
	
}

/** Parses the given string into a encrypted message struct
	@param msg The encrypted message to parse
	@param res pointer to an message_encrypted_o struct to place the results
	@return 1 if sucess, 0 otherwise
*/

/*
struct _message_encrypted {
	unsigned char* encrypted_msg; // The message encrypted with the encrypted key
	unsigned char* encrypted_key; //The encrypted key, which is encrypted with 
		the RSA pub key
	unsigned char* init_vector; // The IV used during encryption
	int encrypted_msg_len; // the length of encrypted_msg
	int encrypted_key_len; // the length of the encrypted key
};
*/

int parse_str_encrypted_msg(char* msg, message_encrypted_o* res) {
	char* tok = strtok(msg, " ");
	//tok is the base64 encoded msg
	int len = Base64decode_len(tok); //length of msg decoded
	res->encrypted_msg = (unsigned char*) malloc(len);
	res->encrypted_msg_len = Base64decode(res->encrypted_msg, tok);
	
	tok = strtok(NULL, " ");
	//tok is the base64 encoded key
	len = Base64decode_len(tok); //length of key decoded
	res->encrypted_key = (unsigned char*) malloc(len);
	res->encrypted_key_len = Base64decode(res->encrypted_key, tok);
	
	tok = strtok(NULL, " ");
	//tok is the base64 encoded key
	len = Base64decode_len(tok); //length of key decoded
	res->init_vector = (unsigned char*) malloc(len);
	res->init_vector_len = Base64decode(res->init_vector, tok);
	
	//msg should be parsed.
	
	return 1;	
}

/** Encrypts the given message with the given public key, and encodes the 
		results in base64
	@param msg cstring containing the message to convert
	@param rsa_ctx A pointer to a rsa_ctx_o struct which contains the rsa context
	@param public_key A pointer to the public key to encrypt with
	@return A cstring containing the Base64 encoded encrypted message, NULL
		if an error occured
*/

char* msg_encrypt_encode(const char* msg, rsa_ctx_o* rsa_ctx, EVP_PKEY* public_key) {
	message_encrypted_o* encrypted_msg = (message_encrypted_o*) 
		malloc(sizeof(message_encrypted_o));
	int ret = client_encrypt_msg(rsa_ctx, msg, public_key, encrypted_msg);
	if (ret) {
		printf("There was an error encrypting the message \n");
		return NULL;
	}
	
	print_msg_struct(encrypted_msg);
	
	char** encoded_msg = (char**) malloc(sizeof(char**));
	ret = parse_encrypted_msg_str(encrypted_msg, encoded_msg);
	if (!ret) {
		printf("Couldnt encoded message \n");
		free(encrypted_msg);
		free(encoded_msg);
		return NULL;
	}
	//TODO: Check if this works correctly.
	char* tmp = *encoded_msg;
	free(encoded_msg);
	return tmp;
}

/** Decodes and decryprs the given message with the specified private key
	@param msg A cstring containing the base64 endoed message to decrypt
	@param rsa_ctx A pointer to the rsa encryption context
	@param private_key A pointer to the private key to decrypt the message with
	@return A cstring containing the decoded and decrypted message, NULL
		if an error occured
*/

char* msg_decode_decrypt(char* msg, rsa_ctx_o* rsa_ctx, EVP_PKEY* private_key) {
	message_encrypted_o* encrypted_msg = (message_encrypted_o*) 
		malloc(sizeof(message_encrypted_o));
		
	int ret = parse_str_encrypted_msg(msg, encrypted_msg);
	if (!ret) {
		printf("Couldn't decode the string \n");
		free(encrypted_msg);
		return NULL;
	}	
	
	
	encrypted_msg->encrypted_msg_len--;
	print_msg_struct(encrypted_msg);
	
	
	return client_decrypt_msg(rsa_ctx, encrypted_msg, private_key);
}


