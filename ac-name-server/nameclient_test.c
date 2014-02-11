//Author: Mark Fitzgibbon
//File is only for the testing of the nameclient functionality. Will not be necessary for final builds.
#include <stdlib.h>

#include "nameclient.h"

int main(int argc, char* argv[]){
	char* address = argv[1];
	char* port = argv[2];

	//call
	contact_server(address, port);
}
