/** Contributers
		Mitchell Caisse
*/

#include <stdlib.h>
#include <string.h>

#include "key_table.h"

/** Counter for the key ids */
int key_id = 0;

/** Creates a new RSA_KEY struct to store the key with the given name
	@param name A cstring containing the name of the key
	@param key A pointer to the key to store
	@return A malloc allocated struct representing the rsa_key
*/
rsa_key_o* create_rsa_key(char* name, EVP_PKEY* key) {
	rsa_key_o* rsa_key = (rsa_key_o*) malloc(sizeof(rsa_key_o));
	rsa_key->key_id = key_id++;
	rsa_key->key = key;
	strncpy(rsa_key->name, name, KT_MAX_KEY_NAME_LEN);
	return rsa_key;
}

/** Creates the hash table to perform key look ups
	@return A pointer to the newly created hash table
*/

GHashTable* key_create_hash_table() {
	return g_hash_table_new_full(g_str_hash, g_str_equal,
		key_hash_free_key, key_hash_free_val);
}

/** Function used to free the key used by the hash table
	@param key Pointer to the key to free
*/

void key_hash_free_key(gpointer key) {
	free(key);
}

/** Function used to free the value used by the hash table
	@param val Pointer to the value to free
*/
void key_hash_free_val(gpointer val) {
	rsa_key_o* rsa_key = (rsa_key_o*) val;
	free(rsa_key->key);
	free(rsa_key);
}

/** Adds the specified rsa_key to the given hash table
	@param hash_table A pointer to the GHashTable to add the key to
	@param name The name of the key to add
	@param key The RSA key to add
	@return A pointer to the rsa_key_o created to store the key
*/

rsa_key_o* key_hash_add(GHashTable* hash_table, char* name, EVP_PKEY* key) {
	//printf("KEYTABLE: Adding Name: %s Key: %x \n", name, key);
	rsa_key_o* rsa_key = create_rsa_key(name, key);
	g_hash_table_insert(hash_table, name, rsa_key);
	return rsa_key;
}

/** Retreives the rsa key with the given name from the specified GHashTable
	@param hash_table A pointer to the GHashTable to lookup the name in
	@param key_name A cstring containing the name of the key to look up
	@param A pointer to the EVP_PKEY with the given name
*/

EVP_PKEY* key_get_by_name(GHashTable* hash_table, char* key_name) {
	rsa_key_o* rsa_key = (rsa_key_o*) g_hash_table_lookup(hash_table, key_name);
	if (rsa_key) {
		return rsa_key->key;
	}
	return NULL;
}

/** Cleans up the GHashTable and frees all of the rsa_key structs
	@param hash_table The hashtable to clean up
*/

void key_hash_cleanup(GHashTable* hash_table) {
	g_hash_table_remove_all(hash_table);
	free(hash_table);
}