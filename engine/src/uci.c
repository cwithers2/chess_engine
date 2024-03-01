#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <debug.h>
#include <getline_noblock.h>
#include <uci.h>
#include <commands.h>
#include <responses.h>
#include <tokenize.h>
#include "commands.c"
#include "responses.c"

/** DEFINITIONS :) **/
#define IS_KEYWORD(kw, val) (strcmp(kw, val) == 0)
#define DELIM " \t\n"
#define CHECK_TOKEN(BUF, CMD) check_token(BUF, CMD, DELIM)
#define COUNT_UNTIL_TOKEN(BUF, CMD) count_until_token(BUF, CMD, DELIM)
#define COUNT_UNTIL_DELIM(BUF) count_until_delim(BUF, DELIM)
#define COUNT_WHILE_DELIM(BUF) count_while_delim(BUF, DELIM)

typedef enum TokenState TokenState;
typedef struct ParseData ParseData;

/** DATA STRUCTURES :) **/
enum TokenState{
	TOKEN_UNDEF,
	TOKEN_ID,
	TOKEN_START,
	TOKEN_GLOB,
	TOKEN_SETOPT,
	TOKEN_VALUE,
	TOKEN_DEFAULT,
	TOKEN_DONE
};

struct ParseData{
	int argc;
	char** argv;
};

/** PRIVATE FORWARD DECLARATIONS :) **/
void free_parsedata(ParseData* data);
int valid_command(char* value);
int valid_response(char* value);
int parse_token(char* buffer, TokenState* state);
int delimit_tokens(char* buffer);
void copy_tokens(ParseData* data, char* buffer);
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
void free_parsedata(ParseData* data){
	free(data->argv);
	data->argc = 0;
	data->argv = NULL;
}

int valid_command(char* value){
	int i;
	debug_print("Validating command: %s\n", value);
	for(i = 0; i < UCI_UI_COMMANDS_COUNT; ++i)
		if(IS_KEYWORD(UCI_UI_COMMANDS[i], value))
			return 1;
	return 0;
}

int valid_response(char* value){
	int i;
	debug_print("Validating response: %s\n", value);
	for(i = 0; i < UCI_UI_RESPONSES_COUNT; ++i)
		if(IS_KEYWORD(UCI_UI_RESPONSES[i], value))
			return 1;
	return 0;
}

int parse_token(char* buffer, TokenState* state){
	int len;
	len = 0;
	switch(*state){
	case TOKEN_START:
		if(!CHECK_TOKEN(buffer, UCI_UI_COMMAND_SETOPTION)){
			*state = TOKEN_SETOPT;
			len = sizeof(UCI_UI_COMMAND_SETOPTION)-1;
			break;
		}
		*state = TOKEN_DEFAULT;
		goto DEFAULT_CASE;
	case TOKEN_GLOB:
		*state = TOKEN_DONE;
		len = strlen(buffer);
		break;
	case TOKEN_SETOPT:
		if(CHECK_TOKEN(buffer, UCI_UI_COMMAND_SETOPTION_ID)){
			*state = TOKEN_ID;
			len = sizeof(UCI_UI_COMMAND_SETOPTION_ID)-1;
			break;
		}
		goto ABORT;
	case TOKEN_ID:
		len = COUNT_UNTIL_TOKEN(buffer, UCI_UI_COMMAND_SETOPTION_VALUE);
		if(len < 0){
			*state = TOKEN_DONE;
			len = -len;
		}else
			*state = TOKEN_VALUE;
		break;
	case TOKEN_VALUE:
		if(CHECK_TOKEN(buffer, UCI_UI_COMMAND_SETOPTION_VALUE)){
			*state = TOKEN_GLOB;
			len = sizeof(UCI_UI_COMMAND_SETOPTION_VALUE)-1;
			break;
		}
		goto ABORT;
	DEFAULT_CASE:
	default:
		len = COUNT_UNTIL_DELIM(buffer);
		if(len < 0)
			len = -len;
		break;
	}
	return len;
	ABORT:
	*state = TOKEN_UNDEF;
	return -strlen(buffer);
}

int delimit_tokens(char* buffer){
	char* ptr;
	char* end;
	int count;
	int len;
	TokenState state;
	ptr = buffer;
	end = buffer + strlen(buffer);
	count = 0;
	state = TOKEN_START;
	while(ptr != end){
		len = COUNT_WHILE_DELIM(buffer);
		if(len < 0)
			break;
		ptr += len;
		len = parse_token(ptr, &state);
		if(len < 0)
			break;
		ptr += len;
		ptr[0] = '\0';
		++count;
		++ptr;
	}
	return count;
}

void copy_tokens(ParseData* data, char* buffer){
	char* ptr;
	int len;
	int i;
	len = 0;
	ptr = buffer;
	for(i = 0; i < data->argc; ++i){
		ptr += len;
		len = COUNT_WHILE_DELIM(buffer);
		ptr += len;
		data->argv[i] = ptr;
		len = strlen(ptr) + 1;
	}
}

int parse(char* buffer, ParseData* data){
	int n;
	n = delimit_tokens(buffer);
	if(!n)
		goto ERROR;
	if(!valid_command(buffer))
		goto ERROR;
	debug_print("%s\n", "Allocating vector.");
	data->argv = (char**)malloc(n*sizeof(char*));
	if(!data->argv){
		debug_print("%s\n", "Failed to allocate.");
		goto ERROR;
	}
	data->argc = n;
	copy_tokens(data, buffer);
	return 1;
	ERROR:
	debug_print("%s\n", "Failed to parse buffer.");
	free_parsedata(data);
	return 0;
}
