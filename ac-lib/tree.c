#include "tree.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

int compareNodes(node* uno, node* dos){
	if(uno == dos){
		return 1;
	}
	else{
		return 0;
	}
}

int addConnection(node* uno, node* dos){
	//add dos to uno
	//allocate space to uno->connections if necessary
	
	printf("%d\n", dos->numConnections);
	
	uno->connections =  realloc(uno->connections, sizeof(node)*(uno->numConnections + 1));
	//add pointer to dos at end
	uno->connections[uno->numConnections] = dos;
	//increase uno's counter
	uno->numConnections = uno->numConnections +1;
					printf("PingA!\n");

	//add uno to dos
	dos->connections = realloc(dos->connections, sizeof(node)*(dos->numConnections + 1));
	//add pointer to dos at end
	printf("PingB!\n");
	dos->connections[dos->numConnections] = uno;
	//increase uno's counter
	
	dos->numConnections = dos->numConnections +1;
					

	return 0;
}



node* combineNodesToGraph(node* group[], int minConnections, int size){
	if(size == 1){
		printf("Ping!!\n");
		//do nothing
	}
	else if(size < (minConnections - 1)){
		//for each node in group[]
		
		int count;
		for(count = 0; count < size; count++){
		//add connection with * minConnections like so
		//1. connect to next cieling(minConnections/4) in array 
		//this creates a looping effect when completed, so all nodes will be halfway done
			int sub;
			for(sub = 1; sub <= minConnections; sub++){
				printf("size %d\n", size);
			//	printf("%d\n",  (group[0+sub])->numConnections);
				if(count + sub > size){
					printf("%p\n", group[0+sub]);
					addConnection(group[count], group[0+sub]);
				}
				else{
					printf("%p\n", group[count+sub]);
					addConnection(group[count], group[count+sub]);
					
				}
			}
			

			/*
			//2. for remainder of minConnections, pick from the far half. Expand choice range for collisions (already on list)
			//each node should have, before this point, ceil(minConnections/2)
			int remPerNode = minConnections - ceil(minConnections/2);
			int subCountB = 0;
			for(subCountB = 0; subCountB < remPerNode; subCountB++){
				if(subCountB != 0){
					if(subCountB % 2 == 0){
					//even number go short
						if(size < (count + (size/2))){
							addConnection(group[count], group[(int) (size - count+(size/2)-ceil(subCountB/2))]);
						}
						else{
							addConnection(group[count], group[(int) (count+(size/2)-ceil(subCountB/2))]);
						}
					}
					else{
					//far number go long
						if(size < (count + (size/2))){
							addConnection(group[count], group[(int) (size - count+(size/2)+ceil(subCountB/2))]);
						}
						else{
							addConnection(group[count], group[(int) (count+(size/2)+ceil(subCountB/2))]);
						}
					}
				}
				else{//first choice here is always opposite on circle
					if(size < (count + (size/2))){
						addConnection(group[count], group[size - count+(size/2)]);
					}
					else{
						addConnection(group[count], group[count+(size/2)]);
					}
				}
			}*/
		}
	}
	else{
	//In cases where minConnections exceeds possible connections per node (approximatley n-1 of group size), ignore minConnections and assume max
	//aka add everyone to everyone else
		int count = 0;
		for(count = 0; count < size; count++){
			int subCount = 0;
			for(subCount = 0; subCount < size; subCount++){
				if(count == subCount){
					//do nothing, same numbers
				}
				else{
					addConnection(group[count], group[subCount]);
				}
			}
		}
		
	}//return last node operated on to act as head

	return group[size-1];
}


int numberOfConnections(node* n){
	return n->numConnections;
}


node* createNode(void* data){
	node* n = (node*) malloc(sizeof(node));
	n->data = data;
	n->numConnections = 0;
	n->connections = malloc(sizeof(node*));

	return n;
}

/** Cleans up the given node and returns a pointer to the data that the node contains
	@param n The node to clean up
	@return A pointer to this nodes data
*/

void* cleanupNode(node* n) {
	void* data = n->data;
	free(n);
	return data;
}
