#include "tree.h"
#include <stdlib.h>

int compareNodes(node uno, node dos){
	if(uno == dos){
		return 1;
	}
	else{
		return 0;
	}
}

int addConnection(node uno, node dos){
	//add dos to uno
	//allocate space to uno->connections if necessary
	uno->connections = realloc(uno->connections, sizeof(node)*(uno->numConnections + 1));
	//add pointer to dos at end
	uno->connections[numConnections] = *dos;
	//increase uno's counter
	uno->numConnections = uno->numConnections +1;

	//add uno to dos
	dos->connections = realloc(dos->connections, sizeof(node)*(dos->numConnections + 1));
	//add pointer to dos at end
	dos->connections[numConnections] = *uno;
	//increase uno's counter
	dos->numConnections = dos->numConnections +1;

	return 0;
}

int removeConnection(node uno, node dos){
	//note: this function intentionally ignores the minConnections value. 

	//check both have connection to each other
	//remove uno from dos
		//remove uno from connections list
		//shift space in array as neccessary
		//decrement dos' counter

	//remove dos from uno
		//remove dos from connections list
		//shift space in array as neccessary
		//decrement uno's counter
}

node combineNodesToGraph(node group[], int minConnections, int size){
	
	if(size < (minConnections - 1)){
		//for each node in group[]
		int count = 0;
		for(count = 0; count < size; count++){
		//add connection with * minConnections like so
		//1. connect to next cieling(minConnections/4) in array 
		//this creates a looping effect when completed, so all nodes will be halfway done
			int leftChain = (int) ceil(minConnections/4);
			int subCount = 1;
			for(subCount = 1; subCount <= leftChain; subCount++){
				if(size < (subCount + count)){ //subCount is looping over the end of the array
					addConnection(group[count], group[size-(count+subCount)]);//later ones in array loop back to start, so for an array of 8. 1->2, 2->3, ... 8->1 for minConnections of 2
				}
				else{
					addConnection(group[count], group[count+subCount]);
				}	
			}
			//2. for remainder of minConnections, pick from the far half. Expand choice range for collisions (already on list)
			//each node should have, before this point, ceil(minConnections/2)
			int remPerNode = minConnections - ceil(minConnections/2);
			int subCountB = 0;
			for(subCountB = 0; subCountB < remPerNode; subCountB++){
				if(subCountB != 0){
					//even number go short

					//far number go long
				}
				else{//first choice here is always opposite on circle
					if(size < (count + (size/2))){
						addConnection(group[count], group[size - count+(size/2)]);
					}
					else{
						addConnection(group[count], group[count+(size/2)]);
					}
				}
			}
		}
	}
	else{
	//In cases where minConnections exceeds possible connections per node (approximatley n-1 of group size), ignore minConnections and assume max
	
		
	}//return last node operated on to act as head

	return group[size-1];
}

int nodeInGraph(node head, node target){

}

int numberOfConnections(node n){
	return n->numConnections;
}

int addNodeToGraph(node head, node target, int minConnections){
	//Assuming graph is built by combineNodesToGraph, random selection of connections should be satisfactory.
}

node createNode(){
	node n;
	n->numConnections = 0;
	n->connections = malloc(sizeof(*n));

	return n;
}