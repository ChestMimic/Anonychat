
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "client.h"
#include "list.h"

/** Adds b as a connection to a
	@param a the client to add the connection to
	@param b The client to add
*/

void add_connection_client(client* a, client* b) {
	list_add(a->connections, b); // add b to a
}


/** Creates a connection graph between all peers in the given list
	@param client_list The list containing all of the connected clients
*/

void create_connection_graph(list* client_list) {		
	int num_clients = list_size(client_list);
	int i;
	for (i=0; i < num_clients; i++) {
		
		client* cur_client = list_item_at(client_list, i);
		pthread_mutex_lock(&(cur_client->connections->mutex));
		
		//remove all elements from the client list
		list_remove_all(cur_client->connections);
		
		//add all neighbors
		
		client* tar_client = list_item_at(client_list, fix_index(i + 1, num_clients));		
		add_connection_client(cur_client, tar_client);
		
		tar_client = list_item_at(client_list, fix_index(i + 2, num_clients));
		add_connection_client(cur_client, tar_client);
		
		tar_client = list_item_at(client_list, fix_index(i - 1, num_clients));
		add_connection_client(cur_client, tar_client);		
		
		tar_client = list_item_at(client_list, fix_index(i - 2, num_clients));
		add_connection_client(cur_client, tar_client);
		
		//add client directly across in the ring
		tar_client = list_item_at(client_list, 
			fix_index(i + num_clients / 2, num_clients));
		add_connection_client(cur_client, tar_client);
		
		//unlock the list mutex
		pthread_mutex_unlock(&(cur_client->connections->mutex));
		
	}
}

/** Adds all other clients to each client in the specified list
	@param clietn_list The list containing all of the connected clients
*/

void create_connection_all(list* client_list) {
	int i;
	for (i = 0;i < list_size(client_list); i++) {
		client* target = list_item_at(client_list, i);
		pthread_mutex_lock(&(target->connections->mutex));
		list_remove_all(target->connections);
		
		int j;
		for (j = 0; j < list_size(client_list); j++) {
			if (j == i) {
				continue;
			}			
			client* to_add = list_item_at(client_list, j);
			add_connection_client(target, to_add);			
		}
		
		pthread_mutex_unlock(&(target->connections->mutex));
		
	}
}

/** Makes sure that the given index does not exceded the array size, and if it does
	loops the index around
	@param index THe index to check
	@param size The size of the array
	@return The new index to use
*/

int fix_index(int index, int size) {
	if (index >= size) {
		index -= size;
	}
	if (index < 0) {
		index = size + index;
	}
	
	return index;
}
