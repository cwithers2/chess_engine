#include <string.h>

int get_token(char* buffer, char** token_start, char* delim){
	char* ptr;
	*token_start = NULL;
	for(ptr = buffer; *ptr; ++ptr){
		if(!*token_start){
			if(strchr(delim, *ptr) != NULL)
				continue;
			*token_start = ptr;
			continue;
		}
		if(strchr(delim, *ptr) != NULL)
			break;
	}
	if(!*token_start)
		return 0;
	return ptr - *token_start;
}
