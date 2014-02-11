#include "msg.h"


/** Determines if the string a starts with the string b,
	@param a cstring a
	@param b cstring b
	@return 1 if a starts with b, 0 otherwie
*/

int str_starts_with(const char* a, const char* b) { 
	return strncmp(a, b, strlen(b)) == 0;
}