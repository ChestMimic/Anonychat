//Author: Mark Fitzgibbon
//File is only for the testing of the nameclient functionality. Will not be necessary for final builds.

#include "nameclient.h"

int main(int argc, char* argv[]){
	int address = argv[1];
	int port = argv[2];

	//call
	contact_server(address, port);
}