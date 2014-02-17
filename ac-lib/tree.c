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

}

int nodeInGraph(node head, node target){

}

int numberOfConnections(node n){
	return n->numConnections;
}

int addNodeToGraph(node head, node target, int minConnections){

}