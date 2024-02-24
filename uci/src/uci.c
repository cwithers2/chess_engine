#include <stdlib.h>
#include <string.h>

#include <debug.h>
#include <getline_noblock.h>
#include <uci.h>
#include <commands.h>
#include <responses.h>

/** DEFINITIONS :) **/
#define _whitespace ' ':case '\t':case '\n'
#define _is_keyword(kw, val) (strcmp(kw, val) == 0)
typedef struct ParseData ParseData;

/** DATA STRUCTURES :) **/
struct ParseData{
	int argc;
	char** argv;
};

/** PRIVATE FORWARD DECLARATIONS :) **/
void free_vector(int argc, char** argv);
void free_parsedata(ParseData* data);
int valid_command(char* value);
int valid_response(char* value);
int is_whitespace(const char ch);
int count_tokens(char* buffer);
int copy_tokens(ParseData* data, char* buffer);
int parse(char* buffer, ParseData* data);

/** PUBILC LIBRARY FUNCTIONS :) **/
int uci_init(){
	int result;
	result = getline_noblock_init();
	return result;
}

void uci_destroy(){
	return;
}

int uci_respond(char* id, int argc, char** argv){
	int i;
	if(!valid_response(id))
		return 1;
	printf("%s", id);
	for(i = 0; i < argc; ++i)
		printf(" %s", argv[i]);
	printf("\n");
	return 0;
}

int uci_poll(int* argc, char*** argv){
	static char buffer[UCI_INPUT_BUFFER_LEN];
	static ParseData data = {0};
	if(data.argc){
		debug_print("%s\n", "Freeing old parsed data.");
		free_parsedata(&data);
	}
	debug_print("%s\n", "Polling for input.");
	if(!getline_noblock(buffer, UCI_INPUT_BUFFER_LEN)){
		debug_print("%s\n", "No input detected.");
		return 0;
	}
	debug_print("%s\n", "Input detected.");
	if(!parse(buffer, &data)){
		debug_print("%s\n", "Unable to parse input.");
		return 0;
	}
	debug_print("%s\n", "Input successfully parsed.");
	*argc = data.argc;
	*argv = data.argv;
	return 1;
}

/** PRIVATE LIBRARY FUNCTIONS :) **/
void free_vector(int argc, char** argv){
	int i;
	debug_print("%s\n", "Freeing vector.");
	for(i = 0; i < argc; ++i){
		free(argv[i]);
	}
	free(argv);
}

void free_parsedata(ParseData* data){
	free_vector(data->argc, data->argv);
	data->argc = 0;
	data->argv = NULL;
}

int valid_command(char* value){
	int i;
	debug_print("Validating command: %s\n", value);
	for(i = 0; i < UCI_UI_COMMANDS_COUNT; ++i)
		if(_is_keyword(UCI_UI_COMMANDS[i], value))
			return 1;
	return 0;
}

int valid_response(char* value){
	int i;
	debug_print("Validating response: %s\n", value);
	for(i = 0; i < UCI_UI_RESPONSES_COUNT; ++i)
		if(_is_keyword(UCI_UI_RESPONSES[i], value))
			return 1;
	return 0;
}

int is_whitespace(const char ch){
	switch(ch){
		case _whitespace:
			return 1;
		default:
			return 0;
	}
}

int count_tokens(char* buffer){
	int ignore;
	int count;
	char* ptr;
	debug_print("%s\n", "Counting tokens.");
	ignore = 1;
	count = 0;
	ptr = buffer;
	while(*ptr){
		if(is_whitespace(*ptr)){
			ignore = 1;
		}else{
			if(ignore)
				++count;
			ignore = 0;
		}
		++ptr;
	}
	debug_print("Counted %d token(s).", count);
	return count;
}

int copy_tokens(ParseData* data, char* buffer){
	int reading;
	int index;
	int length;
	char* ptr;
	char* copy;
	reading = 0;
	ptr = buffer;
	index = 0;
	length = 1;
	debug_print("%s\n", "Copying tokens.");
	while(index < data->argc){
		if(is_whitespace(*ptr)){
			if(reading){
				debug_print("Allocating buffer for token %d.\n", index+1);
				data->argv[index] = (char*)calloc(length, sizeof(char));
				if(!data->argv[index]){
					debug_print("%s\n", "Failed to allocate.");
					data->argc = index;
					return 0;
				}
				memcpy(data->argv[index], copy, length * sizeof(char));
				debug_print("%s\n", "Token copied.");
				++index;
				length = 1;
			}
			reading = 0;
		}else{
			if(!reading)
				copy = ptr;
			else
				++length;
			reading = 1;
		}
		++ptr;
	}
	debug_print("%s\n", "Tokens copied to node.");
	return 1;
}

int parse(char* buffer, ParseData* data){
	int n;
	n = count_tokens(buffer);
	if(!n)
		goto ERROR;
	debug_print("%s\n", "Allocating vector.");
	data->argv = (char**)malloc(n*sizeof(char*));
	if(!data->argv){
		debug_print("%s\n", "Failed to allocate.");
		goto ERROR;
	}
	data->argc = n;
	if(!copy_tokens(data, buffer))
		goto ERROR;
	if(!valid_command(data->argv[0]))
		goto ERROR;
	return 1;
	ERROR:
	debug_print("%s\n", "Failed to parse buffer.");
	free_parsedata(data);
	return 0;
}

#undef _whitespace
#undef _is_keyword
