#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H


#include <pthread.h>



/** Structure representing a list element
*/

struct _list_elm {
	void* val;
	struct _list_elm* next; // the next element in the list, NULL if tail
	struct _list_elm* prev; // the previous element in the list, NULL if head
};

typedef struct _list_elm list_elm;

/** Structure representing a multi threaded list
*/

struct _list {
	list_elm* head; // the head of the list
	list_elm* tail; // the tail of the list
	int size; // the size of the list
	pthread_mutex_t mutex; // the mutex to hold for this list
};

typedef struct _list list;

/** Adds the given value to the list
	@param val A pointer to the value to add
	@return 1 //TODO: Implement error code.
*/

int list_add(list* list, void* val);

/** Removes the given value from the specified list
	@param val The pointer to the item to remove, must be the same as when it was added
	@return The item that was removed, or NULL if the item wasn't found
*/

void* list_remove(list* list, void* val);

/** Returns the size of the list
	@param list The list
	@return The size of the list
*/

int list_size(list* list);

/** Returns the pos'th item in the list
	@param list The list
	@param pos The position of the item in the list to get
	@param The pos'th item in the list
*/

void* list_item_at(list* list, int pos);

/** Creates a list
	@return The created list
*/
list* list_create();

#endif