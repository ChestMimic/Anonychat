#ifndef QUEUE_H
#define QUEUE_H

struct _queueItem{
	node* value;
	struct _queueItem* next;
}

struct _queue{
	struct _queueItem* lst;
	int size; 
}

typedef struct _queueItem queueItem;

typedef struct _queue queue;

queue enqueue(node* n, queue* q);

queue dequeue(node* n, queue* q);

node* breadthFirstSearch(node* n, node* n);

int queueContains(node* n, queue* q);

#endif
