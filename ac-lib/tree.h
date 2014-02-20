#ifndef TREE_H
#define TREE_H

//TODO: When agorithm is satisfactory add support for using clients
struct _node{
	int numConnections;
	struct _node ** connections; 
}

typedef struct _node node

//return 1 if same, 0 else
int compareNodes(node* uno, node* dos);

//Creates a two-way connection between uno and dos
//Return negative on failure
int addConnection(node* uno, node* dos);



//Takes array of nodes and connects them all, with a minimum number of connections per node
//return "head" node
node* combineNodesToGraph(node* group[], int minConnections);

//Return the number of IMMEDIATE connections to a node
int numberOfConnections(node* n);


node createNode();

#endif