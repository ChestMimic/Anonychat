/** Contributers
		Mitchell Caisse
*/

#ifndef __KEY_TABLE_H__
#define __KEY_TABLE_H__

#include <glib.h>
#include <openssl/evp.h>

#define MAX_KEY_NAME_LEN (100)

/** Struct that contains a rsa encryption key
*/

struct _rsa_key {
	int key_id; // the id of the key
	char name[MAX_KEY_NAME_LEN]; // the name of the key
	EVP_PKEY* key; // the key itself
};

typedef struct _rsa_key rsa_key_o;

/** Creates a new RSA_KEY struct to store the key with the given name
	@param name A cstring containing the name of the key
	@param key A pointer to the key to store
	@return A malloc allocated struct representing the rsa_key
*/
rsa_key_o* create_rsa_key(char* name, EVP_PKEY* key);

/** Creates the hash table to perform key look ups
	@return A pointer to the newly created hash table
*/

GHashTable* key_create_hash_table();

/** Function used to free the key used by the hash table
	@param key Pointer to the key to free
*/

void key_hash_free_key(gpointer key);

/** Function used to free the value used by the hash table
	@param val Pointer to the value to free
*/
void key_hash_free_val(gpointer val);

/** Adds the specified rsa_key to the given hash table
	@param hash_table A pointer to the GHashTable to add the key to
	@param name The name of the key to add
	@param key The RSA key to add
	@return A pointer to the rsa_key_o created to store the key
*/

rsa_key_o* key_hash_add(GHashTable* hash_table, char* name, EVP_PKEY* key) ;

/** Retreives the rsa key with the given name from the specified GHashTable
	@param hash_table A pointer to the GHashTable to lookup the name in
	@param key_name A cstring containing the name of the key to look up
	@param A pointer to the EVP_PKEY with the given name
*/

EVP_PKEY* key_get_by_name(GHashTable* hash_table, char* key_name);

/** Cleans up the GHashTable and frees all of the rsa_key structs
	@param hash_table The hashtable to clean up
*/

void key_hash_cleanup(GHashTable* hash_table);


#endif
