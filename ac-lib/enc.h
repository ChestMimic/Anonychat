#ifndef ENC_H
#define ENC_H

#include <openssl/conf.h>
#include <openssl/evp.h>

#include "msg.h"

/** Encryption context struct, contains the contexts for RSA
		encryption and decryption
*/

struct _rsa_ctx {
	EVP_CIPHER_CTX rsa_encrypt_ctx; // encryption context
	EVP_CIPHER_CTX rsa_decrypt_ctx; // decryption context
};

typedef struct _rsa_ctx rsa_ctx_o;

/** Initializes the OpenSSL crypto library for use
*/

void client_initialize_crypto();

/** Cleans up the OpenSSL crypto library after use
*/

void client_cleanup_crypto();

/** Creates an rsa_ctx_o, initializes it, and returns a pointer to it
	@return A pointer to the newly created rsa_ctx, or NULL if an error occured
*/

rsa_ctx_o* client_create_rsa_ctx();

/** Cleans up the specified rsa_ctx
	@param rsa_ctx Pointer to the rsa_ctx_o to clean up
	@return 1 if successful, 0 otherwise
*/

int client_clean_rsa_ctx(rsa_ctx_o* rsa_ctx);

/** Loads a Public key from the file at the path specified
	@param file_path cstring containing the path of the file to load
	@return A pointer to the EVP_PKEY struct representing the public key, or NULL
		if an error occured
*/

EVP_PKEY* client_open_pub_key(char* file_path);


/** Loads a Private key from the file at the path specified
	@param file_path cstring containing the path of the file to load
	@return A pointer to the EVP_PKEY struct representing the private key, or NULL
		if an error occured
*/

EVP_PKEY* client_open_priv_key(char* file_path);



/** Attempts to decypt the given msg
	@param rsa_ctx Pointer to an struct containing the rsa context
	@param msg Pointer a message_encrypted_o which contains the msg, ek, and iv
	@param private_key Pointer to an EVP_PKEY, which contains the private key to decrypt the
		message with
	@return A pointer to the decypted message, or NULL if 
		unable to decrypt
*/

char* client_decrypt_msg(rsa_ctx_o* rsa_ctx, message_encrypted_o* msg, EVP_PKEY* private_key);

/** Encrypt the given msg with the given public key
	@param rsa_ctx A pointer to a struct containing the rsa context
	@param msg The message to encrypt
	@param public_key a cstring containing the public key
	@param res A pointer to a message_encrypted_o struct to place the resulting
		encrypted message, iv, and ek
	@return 0 if successful, error code otherwise
*/

int client_encrypt_msg(rsa_ctx_o* rsa_ctx, const unsigned char* msg, EVP_PKEY* public_key,
	 message_encrypted_o* res);

/** Parses an encrytped message struct into a string
	@param encrypted_msg A pointer to the encrypted_msg_o containing the encrypted message
	@param dest A pointer to a cstring to store the string in, free after use
	@return The lenght of the string
*/

int parse_encrypted_msg_str(message_encrypted_o* encrypted_msg, char** dest);

/** Parses the given string into a encrypted message struct
	@param msg The encrypted message to parse
	@param res pointer to an message_encrypted_o struct to place the results
	@return 1 if sucess, 0 otherwise
*/

int parse_str_encrypted_msg(char* msg, message_encrypted_o* res);

/** Encrypts the given message with the given public key, and encodes the 
		results in base64
	@param msg cstring containing the message to convert
	@param rsa_ctx A pointer to a rsa_ctx_o struct which contains the rsa context
	@param public_key A pointer to the public key to encrypt with
	@return A cstring containing the Base64 encoded encrypted message, NULL
		if an error occured
*/

char* msg_encrypt_encode(const char* msg, rsa_ctx_o* rsa_ctx, EVP_PKEY* public_key);

/** Decodes and decryprs the given message with the specified private key
	@param msg A cstring containing the base64 endoed message to decrypt
	@param rsa_ctx A pointer to the rsa encryption context
	@param private_key A pointer to the private key to decrypt the message with
	@return A cstring containing the decoded and decrypted message, NULL
		if an error occured
*/

char* msg_decode_decrypt(const char* msg, rsa_ctx_o* rsa_ctx, EVP_PKEY* private_key);

/** Prints out an array of unasigned char's as hex
	@param bytes The array of bytes to print out
	@param len The len of the array
*/

void print_hex(unsigned char* bytes, int len);

/** Prints out the items inside of the message_encrypted struct for
		debugging purposes
	@param msg A pointer to the message_encrypted_o struct to prin
*/

void print_msg_struct(message_encrypted_o* msg);

#endif
