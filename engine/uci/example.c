#include <stdio.h>
#include "include/uci.h"

#define RESPOND

int main(int argc, char** argv){
	char buffer[256];
	fgets(buffer, 256, stdin);
#ifdef RESPOND
	if(!uci_write(buffer))
		return 1;
#else
	if(!uci_parse(buffer, &argc, &argv))
		return 1;
	printf("TOKENS: %i\n", argc);
	for(int i = 0; i < argc; ++i)
		printf("%i: %s\n", i, argv[i]);
#endif
	return 0;
}
