#include "client_list.h"

#include <stdlib.h>
#include <stdio.h>

/** Adds the given value to the list
	@param val A pointer to the value to add
	@return 1 //TODO: Implement error code.
*/

int list_add(list* list, void* val) {
	list_elm* elm = (list_elm*) malloc(sizeof(list_elm));
	elm->val = val;
	if (list_size(list) == 0) {
		elm->next = NULL;
		elm->prev = NULL; // only item in the list
		list->head = elm;
		list->tail = elm;
	}
	else {
		list_elm* cur_tail = list->tail;
		cur_tail->next = elm;
		elm->prev = cur_tail;
		list->tail = elm;
	}
	list->size++;
	return 1;
}

/** Removes the given value from the specified list
	@param val The pointer to the item to remove, must be the same as when it was added
	@return The item that was removed, or NULL if the item wasn't found
*/

void* list_remove(list* list, void* val) {
	if (list_size(list) < 1) {
		//can't remove something from an empty list;
		return NULL;
	}
	list_elm* curr_elm = list->head;
	
	while(curr_elm != NULL && curr_elm->val != val) {
		//loop while we haven't found the item, and havent reached end of the list
		curr_elm = curr_elm->next;
	}
	
	void* ret = NULL;
	
	if (curr_elm != NULL) {
		ret = curr_elm->val; // get the value from the item we are removing	
		if (curr_elm == list->head) {
			//we are removing the first item in the list
			list->head = curr_elm->next;
		}
		else {		
			curr_elm->prev->next = curr_elm->next; //set the link before this one, to null.
				//HOWEVER!! if there is only one item in the list. this will segfault
			if (curr_elm->next == NULL) {
				// this was the tail
				list->tail = curr_elm->prev;
			}
		}
		
		list->size--; //decrease the size of the list
		
		free(curr_elm); //free the item element
	}
	
	return ret;
}

/** Returns the size of the list
	@param list The list
	@return The size of the list
*/

int list_size(list* list) {
	return list->size;
}

/** Returns the pos'th item in the list
	@param list The list
	@param pos The position of the item in the list to get
	@return The pos'th item in the list, or NULL if pos >= list_size(list)
*/

void* list_item_at(list* list, int pos) {
	if (pos >= list_size(list) || pos < 0) {
		//pos is invalid
		return NULL;
	}
	list_elm* elm = list->head;
	int i;
	for (i=0;i < pos; i++) {
		elm = elm->next; // advance to the next element
	}
	return elm;
}


/** Creates a list
	@return The created list
*/
list* list_create() {
	list* list_o = (list*) malloc(sizeof(list));
	list_o->size = 0;
	list_o->head = NULL;
	list_o->tail = NULL;
	pthread_mutex_init(&(list_o->mutex), NULL);
	return list_o;
}