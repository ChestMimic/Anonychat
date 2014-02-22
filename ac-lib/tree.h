#ifndef TREE_H
#define TREE_H

//TODO: When agorithm is satisfactory add support for using clients
struct _node{
	int numConnections;
	void* data;
	struct _node ** connections; 
};

typedef struct _node node;

//return 1 if same, 0 else
int compareNodes(node* uno, node* dos);

//Creates a two-way connection between uno and dos
//Return negative on failure
int addConnection(node* uno, node* dos);



//Takes array of nodes and connects them all, with a minimum number of connections per node
//return "head" node
node* combineNodesToGraph(node* group[], int minConnections, int size);

//Return the number of IMMEDIATE connections to a node
int numberOfConnections(node* n);

/** Creates a node with the specified data
	@data A pointer to the data that the node should contain
	@return A pointer to the newly created node struct
*/

node* createNode(void* data);

/** Cleans up the given node and returns a pointer to the data that the node contains
	@param n The node to clean up
	@return A pointer to this nodes data
*/

void* cleanupNode(node* n);

#endif
