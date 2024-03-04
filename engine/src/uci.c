#include <uci.h>
#include <getline_noblock.h>
#include <debug.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

/** DEFINITIONS **/
#define TOKEN_ID(ID) UCI_TOKEN_##ID
#define DELIM " \t\n"
#define COUNT_UNTIL_DELIM(BUF) uci_count_until_delim(BUF, DELIM)
#define COUNT_WHILE_DELIM(BUF) uci_count_while_delim(BUF, DELIM)
#define IS_FLAGGED(NODE) (NODE && uci_token_flags[NODE->id])
#define CASE_GO_SIMPLE(ID)\
case TOKEN_ID(ID):\
	UNFLAG(ID);\
	if(!node->next)\
		parsed = 1;\
	break;
#define CASE_GO_VARIABLE(ID, VAR)\
case TOKEN_ID(ID):\
	UNFLAG(ID);\
	node = node->next;\
	if(!node)\
		goto ABORT;\
	if(sscanf(node->value, "%i", &VAR) != 1)\
		goto ABORT;\
	node->id = UCI_SPEC_TOKEN_STRING;\
	if(!node->next)\
		parsed = 1;\
	break;
#define FLAG(ID) uci_token_flags[TOKEN_ID(ID)] = 1
#define UNFLAG(ID) uci_token_flags[TOKEN_ID(ID)] = 0
#define CALL(ID, CALLER)\
uci_token_flags[TOKEN_ID(ID)] = TOKEN_ID(CALLER)
#define CALLER(ID)\
uci_token_flags[TOKEN_ID(ID)]

typedef struct UCIToken UCIToken;
typedef struct UCITokenList UCITokenList;
typedef struct UCITokenVector UCITokenVector;
typedef unsigned char UCIFlag;

/** DATA TYPES **/
enum UCI_TOKENS{
	UCI_SPEC_TOKEN_UNDEF,
	UCI_SPEC_TOKEN_FLAG,
	UCI_SPEC_TOKEN_STRING,
	#define X(ID, STR) TOKEN_ID(ID),
	#include <xcommands.h>
	#include <xresponses.h>
	#include <xtokens.h>
	#undef X
	UCI_SPEC_TOKEN_COUNT
};

struct UCIToken{
	UCIToken* next;
	int id;
	char* buffer;
	char* value;
};

struct UCITokenList{
	char* buffer;
	UCIToken* head;
	UCIToken* tail;
};

struct UCITokenVector{
	int argc;
	char** argv;
};

/** GLOBALS **/
UCIFlag uci_token_flags[UCI_SPEC_TOKEN_COUNT] = {0};
UCITokenVector uci_assembled_tokens = {0};
UCITokenList uci_tokens;
mtx_t uci_io_mutex;

/** PRIVATE FORWARD DECLARATIONS **/
void uci_flag_commands();
void uci_flag_responses();
void uci_unflag_all();
int uci_string(UCIToken** node);

int uci_count_while_delim(char* buffer, char* delim);
int uci_count_until_delim(char* buffer, char* delim);

char* uci_next_arg(UCIToken** token);
int uci_reassemble_tokens();

UCIToken* uci_new_token(char* value);
void uci_identify(UCIToken* token);
void uci_push(UCIToken* node);

int uci_tokenize();
int uci_parse();

/** PUBLIC FUNCTIONS **/
int uci_init(){
	getline_noblock_init();
	if(mtx_init(&uci_io_mutex, mtx_plain) != thrd_success)
		return 0;
	return 1;
}

void uci_destroy(){
	int i;
	UCIToken* ptr;
	UCIToken* garbage;
	for(i = 0; i < uci_assembled_tokens.argc; ++i)
		free(uci_assembled_tokens.argv[i]);
	free(uci_assembled_tokens.argv);
	ptr = uci_tokens.head;
	while(ptr){
		garbage = ptr;
		ptr = ptr->next;
		free(garbage);
	}
	uci_assembled_tokens.argc = 0;
	uci_assembled_tokens.argv = NULL;
	uci_tokens.head = NULL;
	uci_tokens.tail = NULL;
}

int uci_poll(int* argc, char*** argv){
	char buffer[UCI_INPUT_BUFFER_LEN];
	int result, i;
	mtx_lock(&uci_io_mutex);
	uci_destroy();
	if(!getline_noblock(buffer, UCI_INPUT_BUFFER_LEN)){
		debug_print("%s", "No input available.");
		goto ABORT;
	}
	debug_print("%s", "Input detected.");
	if(!uci_tokenize(buffer))
		goto ABORT;
	uci_flag_commands();
	if(!uci_parse())
		goto ABORT;
	if(!uci_reassemble_tokens())
		goto ABORT;
	*argc = uci_assembled_tokens.argc;
	*argv = uci_assembled_tokens.argv;
	mtx_unlock(&uci_io_mutex);
	return 1;
	ABORT:
	mtx_unlock(&uci_io_mutex);
	uci_destroy();
	return 0;
}

int uci_respond(int argc, char** argv){
	int i;
	char d;
	UCIToken* token;
	mtx_lock(&uci_io_mutex);
	uci_destroy();
	for(i = 0; i < argc; ++i){
		token = uci_new_token(argv[i]);
		if(!token)
			goto ABORT;
		uci_push(token);
	}
	uci_flag_responses();
	if(!uci_parse())
		goto ABORT;
	if(!uci_reassemble_tokens())
		goto ABORT;
	argc = uci_assembled_tokens.argc;
	argv = uci_assembled_tokens.argv;
	d = ' ';
	for(i = 0; i < argc; ++i){
		if(i+1 == argc)
			d = '\0';
		printf("%s%c", argv[i], d);
	}
	mtx_unlock(&uci_io_mutex);
	return 1;
	ABORT:
	mtx_unlock(&uci_io_mutex);
	return 0;
}

/** PRIVATE FUNCTIONS **/
void uci_flag_commands(){
	#define X(ID, STR) FLAG(ID);
	#include <xcommands.h>
	#undef X
	#define X(ID, STR) UNFLAG(ID);
	#include <xresponses.h>
	#include <xtokens.h>
	#undef X
}

void uci_flag_responses(){
	#define X(ID, STR) FLAG(ID);
	#include <xresponses.h>
	#undef X
	#define X(ID, STR) UNFLAG(ID);
	#include <xcommands.h>
	#include <xtokens.h>
	#undef X
}

void uci_unflag_all(){
	#define X(ID, STR) UNFLAG(ID);
	#include <xcommands.h>
	#include <xresponses.h>
	#include <xtokens.h>
	#undef X
}

int uci_string(UCIToken** node){
	if(!*node)
		return 0;
	(*node)->id = UCI_SPEC_TOKEN_STRING;
	if(IS_FLAGGED((*node)->next))
		return 0;
	*node = (*node)->next;
	return 1;
}

int uci_count_while_delim(char* buffer, char* delim){
	char* ptr;
	char* loc;
	for(ptr = buffer; *ptr; ++ptr){
		loc = strchr(delim, *ptr);
		if(!loc)
			return ptr - buffer;
	}
	return -(ptr - buffer);
}

int uci_count_until_delim(char* buffer, char* delim){
	char* ptr;
	char* loc;
	for(ptr = buffer; *ptr; ++ptr){
		loc = strchr(delim, *ptr);
		if(loc)
			return ptr - buffer;
	}
	return -(ptr - buffer);
}

char* uci_next_arg(UCIToken** token){
	size_t size, i;
	char* buffer;
	UCIToken* ptr;
	debug_print("%s", "Constructing next argument.");
	if((*token)->id != UCI_SPEC_TOKEN_STRING){
		debug_print("%s", "Non-stringed token found.");
		size = strlen((*token)->value) + 1;
		buffer = (char*)malloc(size*sizeof(char));
		if(!buffer)
			return NULL;
		strcpy(buffer, (*token)->value);
		*token = (*token)->next;
		return buffer;
	}
	debug_print("%s", "Stringed token(s) found.");
	size = 0;
	for(ptr = *token; ptr && ptr->id == UCI_SPEC_TOKEN_STRING; ptr = ptr->next){
		size += strlen(ptr->value) + 1;
		debug_print("Accumulated discovered size: %li", size);
	}
	debug_print("Allocating buffer of size: %li", size);
	buffer = (char*)malloc(size*sizeof(char));
	if(!buffer)
		return NULL;
	i = 0;
	while(i < size){
		strcpy(buffer+i, (*token)->value);
		i += strlen((*token)->value);
		buffer[i] = ' ';
		++i;
		if(!*token)
			break;
		*token = (*token)->next;
	}
	buffer[size-1] = '\0';
	return buffer;
}

int uci_reassemble_tokens(){
	int i, j;
	UCIToken* ptr;
	debug_print("%s", "Creating vector.");
	ptr = uci_tokens.head;
	while(ptr){
		++uci_assembled_tokens.argc;
		if(ptr->id != UCI_SPEC_TOKEN_STRING){
			ptr = ptr->next;
			continue;
		}
		while(ptr && ptr->id == UCI_SPEC_TOKEN_STRING)
			ptr = ptr->next;
	}
	debug_print("Counted %i argument(s).", uci_assembled_tokens.argc);
	uci_assembled_tokens.argv = (char**)malloc(uci_assembled_tokens.argc*sizeof(char*));
	if(!uci_assembled_tokens.argv)
		return 0;
	ptr = uci_tokens.head;
	for(i = 0; i < uci_assembled_tokens.argc; ++i){
		uci_assembled_tokens.argv[i] = uci_next_arg(&(ptr));
		if(!uci_assembled_tokens.argv[i]){
			uci_assembled_tokens.argc = i;
			return 0;
		}
	}
	return 1;
}

UCIToken* uci_new_token(char* value){
	UCIToken* token;
	debug_print("%s", "Allocating new token.");
	token = (UCIToken*)calloc(1, sizeof(UCIToken));
	if(!token)
		return NULL;
	token->value = value;
	uci_identify(token);
	return token;
}

void uci_identify(UCIToken* token){
	debug_print("%s", "Identifying token.");
	#define X(ID, STR)\
	if(strcmp(token->value, #STR) == 0){\
		debug_print("Token identified as: %s, %i", #ID, TOKEN_ID(ID));\
		token->id = TOKEN_ID(ID);\
		return;\
	}
	#include <xcommands.h>
	#include <xresponses.h>
	#include <xtokens.h>
	#undef X
	debug_print("%s", "Token unidenfied.");
	token->id = UCI_SPEC_TOKEN_UNDEF;
}

void uci_push(UCIToken* node){
	debug_print("%s", "Pushing token onto list.");
	if(!uci_tokens.head){
		uci_tokens.head = node;
		uci_tokens.tail = node;
		return;
	}
	uci_tokens.tail->next = node;
	uci_tokens.tail = node;
}

int uci_tokenize(char* buffer){
	UCIToken* node;
	size_t buffer_size;
	int len;
	char* ptr;
	char* value;
	debug_print("%s", "Tokenizing buffer.");
	for(ptr = buffer; *ptr; ++ptr){
		len = COUNT_WHILE_DELIM(ptr);
		if(len < 0) // no tokens left in buffer
			break;
		debug_print("Skipping forward %i characters.", len);
		ptr += len;
		len = COUNT_UNTIL_DELIM(ptr);
		if(len < 0) // final token found
			len = -len;
		value = ptr;
		ptr += len;
		ptr[0] = '\0';
		debug_print("Creating token for value: %s", value);
		node = uci_new_token(value);
		if(!node)
			goto ABORT;
		uci_push(node);
	}
	debug_print("%s", "Finished tokenization.");
	return 1;
	ABORT:
	debug_print("%s", "Aborting tokenization.");
	return 0;
}

int uci_parse(){
	UCIToken* node;
	int parsed;
	debug_print("%s", "Parsing tokens.");
	for(node = uci_tokens.head, parsed = 0; node && !parsed; node = node->next){
		debug_print("Reviewing token id: %s, %i", node->value, node->id);
		if(!IS_FLAGGED(node))
			goto ABORT;
		switch(node->id){
		/* single word commands */
		case TOKEN_ID(UCI):
		case TOKEN_ID(ISREADY):
		case TOKEN_ID(UCINEWGAME):
		case TOKEN_ID(STOP):
		case TOKEN_ID(PONDERHIT):
		case TOKEN_ID(QUIT):
			parsed = 1;
		/* complex commands */
		/* DEBUG */
		case TOKEN_ID(DEBUG):
			uci_unflag_all();
			FLAG(DEBUG_ON);
			FLAG(DEBUG_OFF);
			break;
		case TOKEN_ID(DEBUG_ON):
		case TOKEN_ID(DEBUG_OFF):
			UNFLAG(DEBUG_ON);
			UNFLAG(DEBUG_OFF);
			parsed = 1;
			break;
		/* SETOPTION */
		case TOKEN_ID(SETOPTION):
			uci_unflag_all();
			CALL(NAME, SETOPTION);
			break;
		case TOKEN_ID(NAME):
			node = node->next;
			if(!node)
				goto ABORT;
			if(CALLER(NAME) == TOKEN_ID(SETOPTION))
				FLAG(SETOPTION_VALUE);
			UNFLAG(NAME);
			while(uci_string(&node));
			if(!node)
				parsed = 1;
			break;
		case TOKEN_ID(SETOPTION_VALUE):
			UNFLAG(SETOPTION_VALUE);
			node = node->next;
			if(!node)
				goto ABORT;
			UNFLAG(NAME);
			while(uci_string(&node));
			parsed = 1;
			break;
		/* GO */
		case TOKEN_ID(GO):
			uci_unflag_all();
			FLAG(GO_SEARCHMOVES);
			FLAG(GO_PONDER);
			FLAG(GO_WTIME);
			FLAG(GO_BTIME);
			FLAG(GO_WINC);
			FLAG(GO_BINC);
			FLAG(GO_MOVESTOGO);
			FLAG(GO_DEPTH);
			FLAG(GO_NODES);
			FLAG(GO_MATE);
			FLAG(GO_MOVETIME);
			FLAG(GO_INFINITE);
			break;

		CASE_GO_SIMPLE(GO_PONDER);
		CASE_GO_SIMPLE(GO_INFINITE);
		int i;
		CASE_GO_VARIABLE(GO_WTIME,     i);
		CASE_GO_VARIABLE(GO_BTIME,     i);
		CASE_GO_VARIABLE(GO_WINC,      i);
		CASE_GO_VARIABLE(GO_BINC,      i);
		CASE_GO_VARIABLE(GO_MOVETIME,  i);
		CASE_GO_VARIABLE(GO_MOVESTOGO, i);
		CASE_GO_VARIABLE(GO_DEPTH,     i);
		CASE_GO_VARIABLE(GO_NODES,     i);
		CASE_GO_VARIABLE(GO_MATE,      i);

		case TOKEN_ID(GO_SEARCHMOVES):
			UNFLAG(GO_SEARCHMOVES);
			node = node->next;
			if(!node)
				goto ABORT;
			while(uci_string(&node)){
			/*TODO verify moves are long algebraic*/
			}
			if(!node)
				parsed = 1;
			break;
		/* POSITION */
		case TOKEN_ID(POSITION):
			uci_unflag_all();
			FLAG(POSITION_FEN);
			FLAG(POSITION_STARTPOS);
			break;
		case TOKEN_ID(POSITION_STARTPOS):
			UNFLAG(POSITION_STARTPOS);
			UNFLAG(POSITION_FEN);
			break;
		case TOKEN_ID(POSITION_FEN):
			UNFLAG(POSITION_STARTPOS);
			UNFLAG(POSITION_FEN);
			node = node->next;
			if(!node)
				goto ABORT;
			FLAG(POSITION_MOVES);
			while(uci_string(&node)){
			/*TODO verify tokens are fen string*/
			}
			break;
		case TOKEN_ID(POSITION_MOVES):
			UNFLAG(POSITION_MOVES);
			node = node->next;
			if(!node)
				goto ABORT;
			while(uci_string(&node)){
			/*TODO verify moves are long algebraic */
			}
			parsed = 1;
			break;
		/* REGISTER */
		case TOKEN_ID(REGISTER):
			uci_unflag_all();
			FLAG(REGISTER_LATER);
			CALL(NAME, REGISTER);
			FLAG(REGISTER_CODE);
			break;
		case TOKEN_ID(REGISTER_LATER):
			parsed = 1;
			break;
		case TOKEN_ID(REGISTER_CODE):
			UNFLAG(REGISTER_CODE);
			UNFLAG(REGISTER_LATER);
			node = node->next;
			if(!node)
				goto ABORT;
			while(uci_string(&node));
			if(!node)
				parsed = 1;
			break;
		default:
			debug_print("%s", "Unhandled token.");
			break;
		}
		if(!node)
			break;
	}
	if(node)
		goto ABORT;
	if(!parsed)
		goto ABORT;
	debug_print("%s", "Finished parsing.");
	return 1;
	ABORT:
	debug_print("%s", "Aborting parsing.");
	return 0;
}

