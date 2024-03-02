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
	TOKEN_UNDEF    = 0x0000,
	TOKEN_START    = 0x0001,
	TOKEN_DEFAULT  = 0x0003,
	TOKEN_GLOB     = 0x0006,
	TOKEN_SETOPT   = 0x0002,
	TOKEN_ID       = 0x0004,
	TOKEN_VALUE    = 0x0008,
	TOKEN_DEBUG    = 0x000A,
	TOKEN_POS      = 0x000C,
	TOKEN_FEN_STR  = 0x000E,
	TOKEN_POS_SET  = 0x0005,
	TOKEN_MOVES    = 0x0007,
	TOKEN_GO       = 0x000B,
	TOKEN_DONE     = 0x1111
};

struct ParseData{
	char buffer[UCI_INPUT_BUFFER_LEN];
	int argc;
	char** argv;
};

/** PRIVATE FORWARD DECLARATIONS :) **/
void free_parsedata(ParseData* data);
int valid_command(char* value);
int valid_response(char* value);
int parse_token(char* buffer, TokenState* state);
int delimit_buffer(char* buffer);
void copy_tokens(ParseData* data);
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

#define REROUTE_TOKEN(TOK, STATE)\
if(CHECK_TOKEN(buffer, TOK)){\
	debug_print("%s\n", "Matched token: " #TOK);\
	*state = STATE;\
	len = sizeof(TOK)-1;\
	break;\
}
int parse_token(char* buffer, TokenState* state){
	int len;
	len = 0;
	switch(*state){
	case TOKEN_START:
		REROUTE_TOKEN(UCI_UI_COMMAND_SETOPTION, TOKEN_SETOPT);
		REROUTE_TOKEN(UCI_UI_COMMAND_DEBUG, TOKEN_DEBUG);
		REROUTE_TOKEN(UCI_UI_COMMAND_POSITION, TOKEN_POS);
		REROUTE_TOKEN(UCI_UI_COMMAND_GO, TOKEN_GO);
		*state = TOKEN_DEFAULT;
		goto DEFAULT;
	case TOKEN_GLOB:
		*state = TOKEN_DONE;
		len = strlen(buffer);
		break;
	case TOKEN_SETOPT:
		REROUTE_TOKEN(UCI_UI_COMMAND_SETOPTION_ID, TOKEN_ID);
		goto ABORT;
	case TOKEN_ID:
		len = COUNT_UNTIL_TOKEN(buffer, UCI_UI_COMMAND_SETOPTION_VALUE);
		if(len < 0){
			*state = TOKEN_DONE;
			len = -len;
		}else{
			*state = TOKEN_VALUE;
			len -= 1;
		}
		break;
	case TOKEN_VALUE:
		REROUTE_TOKEN(UCI_UI_COMMAND_SETOPTION_VALUE, TOKEN_GLOB);
		goto ABORT;
	case TOKEN_DEBUG:
		REROUTE_TOKEN(UCI_UI_COMMAND_DEBUG_ON, TOKEN_DONE);
		REROUTE_TOKEN(UCI_UI_COMMAND_DEBUG_OFF, TOKEN_DONE);
		goto ABORT;
	case TOKEN_POS:
		REROUTE_TOKEN(UCI_UI_COMMAND_POSITION_FEN, TOKEN_FEN_STR);
		REROUTE_TOKEN(UCI_UI_COMMAND_POSITION_STARTPOS, TOKEN_POS_SET);
		goto ABORT;
	case TOKEN_FEN_STR:
		//TODO validate fen string
		*state = TOKEN_POS_SET;
		goto DEFAULT;
	case TOKEN_POS_SET:
		REROUTE_TOKEN(UCI_UI_COMMAND_POSITION_MOVES, TOKEN_MOVES);
		goto ABORT;
	case TOKEN_MOVES:
		//TODO validate move
		goto DEFAULT;
	case TOKEN_GO:
		//TODO REROUTE TOKENS
		goto DEFAULT;
	DEFAULT:
	default:
		len = COUNT_UNTIL_DELIM(buffer);
		if(len < 0)
			len = -len;
		break;
	}
	debug_print("Token of size %i found in: %s\n", len, buffer);
	return len;
	ABORT:
	debug_print("%s\n", "Aborting.");
	*state = TOKEN_UNDEF;
	return -strlen(buffer);
}

int delimit_buffer(char* buffer){
	char* ptr;
	int count;
	int len;
	TokenState state;
	debug_print("%s\n", "Delimiting tokens.");
	ptr = buffer;
	count = 0;
	state = TOKEN_START;
	while(*ptr){
		len = parse_token(ptr, &state);
		if(len <= 0){
			debug_print("%s\n", "Aborting.");
			count = 0;
			break;
		}
		debug_print("Skipping forward %i characters.\n", len);
		ptr += len;
		ptr[0] = '\0';
		++count;
		++ptr;
	}
	return count;
}

void copy_tokens(ParseData* data){
	char* ptr;
	int len;
	int i;
	debug_print("Copying %i tokens.\n", data->argc);
	len = 0;
	ptr = data->buffer;
	for(i = 0; i < data->argc; ++i){
		ptr += len;
		len = COUNT_WHILE_DELIM(ptr);
		debug_print("Skipping forward %i characters.\n", len);
		ptr += len;
		debug_print("Head token: %s\n", ptr);
		data->argv[i] = ptr;
		len = strlen(ptr) + 1;
		debug_print("Skipping forward %i characters.\n", len);
	}
}

void copy_buffer(char* buffer, ParseData* data){
	char* ptr;
	int len, i;
	i = 0;
	for(ptr = buffer; *ptr; ++ptr){
		len = COUNT_WHILE_DELIM(ptr);
		if(len < 0) // nothing left to copy
			break;
		ptr += len;
		len = COUNT_UNTIL_DELIM(ptr);
		if(len < 0) // we read the rest of the buffer
			len = -len;
		memcpy(data->buffer+i, ptr, len);
		data->buffer[i+len] = ' ';
		i += len + 1;
		ptr += len;
	}
	data->buffer[i] = '\0';
}

int parse(char* buffer, ParseData* data){
	copy_buffer(buffer, data);
	data->argc = delimit_buffer(data->buffer);
	if(!valid_command(data->buffer)){
		debug_print("Command '%s' is not valid\n", buffer);
		goto ABORT;
	}
	data->argv = (char**)malloc(data->argc*sizeof(char*));
	if(!data->argv){
		debug_print("%s\n", "Failed to allocate vector.");
		goto ABORT;
	}
	copy_tokens(data);
	return 1;
	ABORT:
	free_parsedata(data);
	return 0;
}
