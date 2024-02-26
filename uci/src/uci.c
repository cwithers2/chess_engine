#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <debug.h>
#include <getline_noblock.h>
#include <uci.h>
#include <src/commands.h>
#include <src/responses.h>

/** DEFINITIONS :) **/
#define _whitespace ' ':case '\t':case '\n'
#define _is_keyword(kw, val) (strcmp(kw, val) == 0)

typedef enum{
	TOKEN_UNDEF,
	TOKEN_NORMAL,
	TOKEN_SETOPT,
	TOKEN_ID,
	TOKEN_VALUE,
}token_t;

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
int is_setoption(char* buffer);
int is_setoption_id(char* buffer);
int is_setoption_value(char* buffer);
int next_token(char* buffer, char** token_start, token_t* type);
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

int is_setoption(char* buffer){
	static size_t len = strlen(UCI_UI_COMMAND_SETOPTION);
	if(memcmp(buffer, UCI_UI_COMMAND_SETOPTION, len) != 0)
		return 0;
	if(!is_whitespace(buffer[len]))
		return 0;
	return 1;
}
int is_setoption_id(char* buffer){
	static size_t len = strlen(UCI_UI_COMMAND_SETOPTION_ID);
	if(memcmp(buffer, UCI_UI_COMMAND_SETOPTION_ID, len) != 0)
		return 0;
	if(!is_whitespace(buffer[len]))
		return 0;
	return 1;
}

int is_setoption_value(char* buffer){
	static size_t len = strlen(UCI_UI_COMMAND_SETOPTION_VALUE);
	if(memcmp(buffer, UCI_UI_COMMAND_SETOPTION_VALUE, len) != 0)
		return 0;
	if(!is_whitespace(buffer[len]))
		return 0;
	return 1;
}

int next_token(char* buffer, char** token_start, token_t* type){
	static token_t normal = TOKEN_NORMAL;
	int length;
	char* ptr;
	char* keyword;
	*token_start = NULL;
	for(ptr = buffer; *ptr; ++ptr){
		if(!(*token_start)){
			if(is_whitespace(*ptr))
				continue;
			*token_start = ptr;
			continue;
		}
		if(*type == TOKEN_VALUE){
			length = strlen(*token_start)-1;
			return length;
		}
		if(!is_whitespace(*ptr))
			continue;
		if(*type == TOKEN_NORMAL)
			break;
		if(!*type){
			if(is_setoption(*token_start))
				*type = TOKEN_SETOPT;
			else
				*type = TOKEN_NORMAL;
			break;
		}
		if(*type == TOKEN_SETOPT && is_setoption_id(*token_start)){
			*type = TOKEN_ID;
			break;
		}
		if(*type == TOKEN_ID && is_setoption_value(*token_start)){
			*type = TOKEN_VALUE;
			break;
		}
		if(*type != TOKEN_ID){
			debug_print("%s\n", "Garbage detected. Expected token 'id'.");
			break;
		}
		length = next_token(ptr, &keyword, &normal);
		if(is_setoption_value(keyword))
			break;		
	}
	if(!*token_start)
		*token_start = ptr;
	return ptr - *token_start;
}

int count_tokens(char* buffer){
	int count;
	int length;
	char* head;
	char* token_start;
	token_t type;
	count = 0;
	head = buffer;
	type = TOKEN_UNDEF;
	debug_print("%s\n", "Counting tokens.");
	while(1){
		length = next_token(head, &token_start, &type);
		if(!length)
			break;
		++count;
		head = token_start + length;
	}
	debug_print("Counted %d token(s).", count);
	return count;
}

int copy_tokens(ParseData* data, char* buffer){
	int length;
	int i;
	char* head;
	char* token_start;
	token_t type;
	head = buffer;
	type = TOKEN_UNDEF;
	debug_print("%s\n", "Copying tokens.");
	for(i = 0; i < data->argc; ++i){
		length = next_token(head, &token_start, &type);
		debug_print("Allocating buffer for token %d.\n", i+1);
		data->argv[i] = (char*)calloc(length, sizeof(char));
		if(!data->argv[i]){
			debug_print("%s\n", "Failed to allocate.");
			data->argc = i;
			return 0;
		}
		memcpy(data->argv[i], token_start, length);
		head = token_start + length;
	}
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
