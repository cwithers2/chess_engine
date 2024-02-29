#include <tokenize.h>
#include <string.h>

int count_until_token(char* buffer, char* token, char* delim){
	char* ptr;
	char* end;
	int len;
	ptr = buffer;
	end = buffer + strlen(buffer);
	len = 0;
	while(ptr != end){
		len = count_while_delim(ptr, delim);
		if(len < 0)//we read the entire string without finding the token
			return -(end - buffer);
		ptr += len;
		if(check_token(ptr, token, delim))
			break;
		len = count_until_delim(ptr, delim);
		if(len < 0)
			return -(end - buffer);
		ptr += len;
	}
	return ptr - buffer;
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
			return ptr - buffer;
	}
	return -(ptr - buffer);
}

int count_until_delim(char* buffer, char* delim){
	char* ptr;
	char* loc;
	for(ptr = buffer; *ptr; ++ptr){
		loc = strchr(delim, *ptr);
		if(loc)
			return ptr - buffer;
	}
	return -(ptr - buffer);
}
