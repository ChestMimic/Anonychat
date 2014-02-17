#ifndef TREE_H
#define TREE_H

//TODO: When agorithm is satisfactory add support for using clients
struct _node{
	int numConnections;
	struct _node ** connections; 
}

typedef struct _node node

//return 1 if same, 0 else
int compareNodes(node uno, node dos);

//Creates a two-way connection between uno and dos
//Return negative on failure
int addConnection(node uno, node, dos);

//Disconnects a two way connection between two nodes
//if these two were already not connected, return sucess
//return negative on failure
int removeConnection(node uno, node dos);

//Takes array of nodes and connects them all, with a minimum number of connections per node
//return "head" node
node combineNodesToGraph(node group[], int minConnections);

//Takes target and looks to see if it is, in any way connected to head
//If yes, return number of hops(matching returns 0, one hop is 1, etc)
//If no, return -1
int nodeInGraph(node head, node target);

//Return the number of IMMEDIATE connections to a node
int numberOfConnections(node n);

//adds a node into a graph.
//
int addNodeToGraph(node head, node target, int minConnections);

#endif