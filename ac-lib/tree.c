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

int addConnection(node uno, node, dos){
	//add dos to uno
		//allocate space to uno->connections if necessary
		//add pointer to dos at end
	//increase uno's counter

	//add uno to dos
	//increase dos' counter

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

node combineNodesToGraph(node group[], int minConnections){
	//for each node in group[]
		//add connection with * minConnections like so
			//1. connect to next cieling(minConnections/2) in array 
				//later ones in array loop back to start, so for an array of 8. 1->2, 2->3, ... 8->1 for minConnections of 2
			//2. for remainder of minConnections, randomly pick from the far half. Expand choice range for collisions (already on list)
	//In cases where minConnections exceeds possible connections per node (approximatley n-1 of group size), ignore and assume max
	//return last node operated on to act as head
}

int nodeInGraph(node head, node target){

}

int numberOfConnections(node n){
	return n->numConnections;
}

int addNodeToGraph(node head, node target, int minConnections){
	//Assuming graph is built by combineNodesToGraph, random selection of connections should be satisfactory.
}