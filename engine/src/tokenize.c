#include <tokenize.h>
#include <string.h>

int count_until_token(char* buffer, char* token, char* delim){
	int len;
	int val;
	len = count_while_delim(buffer, delim);
	if(len < 0)
		return len;
	while(!check_token(buffer, token, delim)){
		val = count_until_delim(buffer + len, delim);
		if(val < 0){
			len = -len + val;
			break;
		}
		len += val;
		val = count_while_delim(buffer + len, delim);
		if(val < 0){
			len = -len + val;
			break;
		}
		len += val;
	}
	return len;
}

int check_token(char* buffer, char* token, char* delim){
	int len;
	len = strlen(token);
	if(strlen(buffer) < len)
		return 0;
	if(memcmp(buffer, token, len) != 0)
		return 0;
	if(!strchr(delim, buffer[len]) || !buffer[len])
		return 0;
	return 1;
}

int count_while_delim(char* buffer, char* delim){
	char* ptr;
	char* loc;
	for(ptr = buffer; *ptr; ++ptr){
		loc = strchr(delim, *ptr);
		if(!loc)
			return loc - buffer;
	}
	return -(ptr - buffer);
}

int count_until_delim(char* buffer, char* delim){
	char* ptr;
	char* loc;
	for(ptr = delim; *ptr; ++ptr){
		loc = strchr(buffer, *ptr);
		if(loc)
			return loc - buffer;
	}
	return -(ptr - buffer);
}

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
