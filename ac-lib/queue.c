#include 'queue.h'
#include 'tree.h'

queue enqueue(node* n, queue* q){
	if(q->lst == NULL){
		//start adding here
		queueItem qI = (queueItem*) malloc(sizeof(queueItem));
		qI->value = n;
		qI->next = NULL;
		q->lst = qI;
	}
	else{
		//queue has items, just follow to end and add
		//look to make sure node is not already on list
	}
}

queueItem dequeue(queue* q){
	if(q->lst == NULL){
		return 0;
	}
	else{
		return q->lst;

	}
	
}

node* breadthFirstSearch(node* alpha, node* target){
	//Followed pseudocode from http://en.wikipedia.org/wiki/Breadth-first_search

	//create queue
	//add alpha to queue
	// while Q is not empty,
		//dequeue top element
		//if match, 
			//return element
		//else
		//for all the connections element has,
			//queue new items (check performed by enqueue())
	//if queue becomes empty, target not found
		//return null

}

