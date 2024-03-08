#include <uci.h>
#include <debug.h>

#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <threads.h>

/** DEFINITIONS **/
#define DELIM " \t\n"
#define OUTPUT_BUFFER_SIZE 256

#define TABLE                     uci_token_table
#define TAB_ACCEPT(ID, PROTOCOL)  TABLE[ID] =  PROTOCOL
#define TAB_REJECT(ID)            TABLE[ID] =  SPEC_UNDEF
#define TAB_CALLER(ID)            TABLE[ID]
#define TAB_CALLED(ID, PROTOCOL) (TABLE[ID] == PROTOCOL)
#define TAB_DIRECT(ID, NEW_ID)\
do{\
	TABLE[NEW_ID] = TABLE[ID];\
	TABLE[ID]  = SPEC_UNDEF;\
}while(0)

typedef struct UCIToken UCIToken;
typedef unsigned char UCITokenID;

/** DATA STRUCTURES **/
struct UCIToken{
	UCIToken* next;
	UCITokenID id;
	char* value;
};
enum UCI_TOKENS{
	/* For anonymous callers */
	SPEC_UNDEF,
	SPEC_FLAG,
	/* Data types */
	TYPE_STRING,
	TYPE_NUMBER,
	TYPE_FEN,
	TYPE_MOVE,
	/* Keywords */
	#define X(ID, STR) ID,
	#include <x/uci/commands.h>
	#include <x/uci/responses.h>
	#include <x/uci/tokens.h>
	#undef X
	SPEC_COUNT
};

/** GLOBALS **/
mtx_t uci_table_lock;
UCITokenID TABLE[SPEC_COUNT];

/** PRIVATE FORWARD DECLARES **/
void* uci_alloc(size_t size);
UCIToken* uci_new_token();
//int uci_iter_uncalled(UCIToken** node);
void uci_free_tokens(UCIToken* tokens);
char* uci_next_vector(UCIToken** token);

int uci_is_number(char* value);
int uci_is_fen(char* value);
int uci_is_move(char* value);

int uci_lock();
void uci_unlock();

void uci_set_input_mode();
void uci_set_output_mode();
void uci_clear_table();

size_t uci_count_while_delim(char* buffer);
size_t uci_count_until_delim(char* buffer);

UCIToken* uci_tokenizer(char* buffer);
int uci_lexer(UCIToken* tokens);
int uci_parser(UCIToken* tokens);
int uci_vectorize(UCIToken* tokens, int* argc, char*** argv);

/** PUBLIC LIBRARY FUNCTIONS **/
int uci_init(){
	return mtx_init(&uci_table_lock, mtx_plain);
}

void uci_destroy(){
	mtx_destroy(&uci_table_lock);
}

int uci_write(char* fmt, ...){
	char buffer[OUTPUT_BUFFER_SIZE];
	UCIToken* tokens;
	va_list args;
	int result;
	if(!uci_lock())
		goto ABORT;
	va_start(args, fmt);
	result = vsprintf(buffer, fmt, args);
	va_end(args);
	if(result < 0)
		goto ABORT;
	uci_set_input_mode();
	tokens = uci_tokenizer(buffer);
	if(!tokens)
		goto ABORT;
	if(!uci_lexer(tokens))
		goto ABORT;
	if(!uci_parser(tokens))
		goto ABORT;
	printf("%s", buffer);
	uci_unlock();
	debug_print("%s", "Finished write.");
	return 1;
	ABORT:
	uci_unlock();
	debug_print("%s", "Aborting write.");
	return 0;
}

int uci_parse(char* buffer, int* argc, char*** argv, UCI_PARSE_MODE mode){
	UCIToken* tokens;
	if(!uci_lock())
		goto ABORT;
	if(mode != UCI_PARSE_OUTPUT)
		uci_set_input_mode();
	else
		uci_set_output_mode();
	tokens = uci_tokenizer(buffer);
	if(!tokens)
		goto ABORT;
	if(!uci_lexer(tokens))
		goto ABORT;
	if(!uci_parser(tokens))
		goto ABORT;
	if(!uci_vectorize(tokens, argc, argv))
		goto ABORT;
	uci_unlock();
	debug_print("%s", "Finished parse.");
	uci_free_tokens(tokens);
	return 1;
	ABORT:
	uci_unlock();
	uci_free_tokens(tokens);
	debug_print("%s", "Aborting parse.");
	return 0;
}

void uci_free_vector(int argc, char** argv){
	int i;
	for(i = 0; i < argc; ++argc)
		free(argv[i]);
	free(argv);
}

/** PRIVATE LIBRARY FUNCTIONS **/
void* uci_alloc(size_t size){
	void* data;
	debug_print("Allocating data of size %li.", size);
	data = malloc(size);
	if(!data)
		goto ERROR;
	debug_print("%s", "Successful allocation.");
	return data;
	ERROR:
	debug_print("%s", "Unable to allocate data.");
	return NULL;
}

UCIToken* uci_new_token(){
	UCIToken* token;
	size_t len;
	debug_print("%s", "Creating token.");
	token = uci_alloc(sizeof(UCIToken));
	if(!token)
		return NULL;
	token->id = SPEC_UNDEF;
	return token;
}

/*USAGE:
	do{
		//do stuff here
	}while(uci_iter_uncalled(&node));
*/
/*int uci_iter_uncalled(UCIToken** node){
	if(!*node)
		return 0;
	if(CALLED((*node)->next->id))
		return 0;
	(*node) = (*node)->next;
	return 1;
}*/

void uci_free_tokens(UCIToken* tokens){
	UCIToken* garbage;
	while(tokens){
		garbage = tokens;
		tokens = tokens->next;
		free(garbage->value);
		free(garbage);
	}
}

char* uci_next_vector(UCIToken** token){
	size_t size, i;
	char* buffer;
	UCIToken* ptr;
	debug_print("%s", "Constructing next vector.");
	if((*token)->id != TYPE_STRING){
		debug_print("%s", "Non-stringed token found.");
		size = strlen((*token)->value) + 1;
		buffer = uci_alloc(size*sizeof(char));
		if(!buffer)
			goto ABORT;
		strcpy(buffer, (*token)->value);
		*token = (*token)->next;
		return buffer;
	}
	debug_print("%s", "Stringed token(s) found.");
	size = 0;
	for(ptr = *token; ptr && ptr->id == TYPE_STRING; ptr = ptr->next){
		size += strlen(ptr->value) + 1;
		debug_print("Accumulated discovered size: %li", size);
	}
	buffer = uci_alloc(size*sizeof(char));
	if(!buffer)
		goto ABORT;
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
	ABORT:
	free(buffer);
	return NULL;
}

int uci_is_number(char* value){
	int i;
	return sscanf(value, "%i", &i);
}

int uci_is_fen(char* value){
	return 1;
}

int uci_is_move(char* value){
	return 1;
}

int uci_lock(){
	if(mtx_lock(&uci_table_lock) != thrd_success){
		debug_print("%s", "Failed to lock table access.");
		return 0;
	}
	return 1;
}

void uci_unlock(){
	if(mtx_unlock(&uci_table_lock) != thrd_success)
		debug_print("%s", "Failed to unlock table access.");
}

void uci_set_input_mode(){
	#define X(ID, STR) TAB_ACCEPT(ID, SPEC_FLAG);
	#include <x/uci/commands.h>
	#undef X
	#define X(ID, STR) TAB_REJECT(ID);
	#include <x/uci/responses.h>
	#include <x/uci/tokens.h>
	#undef X
}

void uci_set_output_mode(){
	#define X(ID, STR) TAB_ACCEPT(ID, SPEC_FLAG);
	#include <x/uci/responses.h>
	#undef X
	#define X(ID, STR) TAB_REJECT(ID);
	#include <x/uci/commands.h>
	#include <x/uci/tokens.h>
	#undef X

}

void uci_clear_table(){
	#define X(ID, STR) TAB_REJECT(ID);
	#include <x/uci/commands.h>
	#include <x/uci/responses.h>
	#include <x/uci/tokens.h>
	#undef X
}

size_t uci_count_while_delim(char* buffer){
	char* ptr;
	char* loc;
	for(ptr = buffer; *ptr; ++ptr){
		loc = strchr(DELIM, *ptr);
		if(!loc)
			return ptr - buffer;
	}
	return -(ptr - buffer);
}

size_t uci_count_until_delim(char* buffer){
	char* ptr;
	char* loc;
	for(ptr = buffer; *ptr; ++ptr){
		loc = strchr(DELIM, *ptr);
		if(loc)
			return ptr - buffer;
	}
	return -(ptr - buffer);
}

UCIToken* uci_tokenizer(char* buffer){
	UCIToken head;
	UCIToken* curr;
	UCIToken* node;
	size_t len;
	char* ptr;
	char* value;
	debug_print("%s", "Tokenizing.");
	curr = &head;
	for(ptr = buffer; *ptr; ++ptr){
		curr->next = NULL;
		len = uci_count_while_delim(ptr);
		if(len < 0) // no tokens left in buffer
			break;
		debug_print("Skipping forward %li characters.", len);
		ptr += len;
		//len = uci_is_fen(ptr);
		if(!len)
			len = uci_count_until_delim(ptr);
		if(len < 0) // final token found
			len = -len;
		debug_print("Found token of length %li.", len); 
		node = uci_new_token();
		if(!node)
			goto ABORT;
		node->value = uci_alloc(len+1);
		if(!node->value){
			free(node);
			goto ABORT;
		}
		//copy string
		memcpy(node->value, ptr, len+1);
		node->value[len] = '\0';
		//advance
		curr->next = node;
		curr = node;
		ptr += len;
	}
	debug_print("%s", "Finished tokenizing.");
	return head.next;
	ABORT:
	debug_print("%s", "Aborting tokenizing.");
	uci_free_tokens(head.next);
	return NULL;
}

#define LEXER_ASSIGN(ID)\
do{\
debug_print("Token indentified: %s, %i", #ID, ID);\
ptr->id = ID;\
}while(0)
int uci_lexer(UCIToken* tokens){
	UCIToken* ptr;
	debug_print("%s", "Lexing.");
	ptr = tokens;
	for(ptr = tokens; ptr; ptr = ptr->next){
		#define X(ID, STR)\
		if(strcmp(ptr->value, #STR) == 0){\
			LEXER_ASSIGN(ID);\
			continue;\
		}
		#include <x/uci/commands.h>
		#include <x/uci/responses.h>
		#include <x/uci/tokens.h>
		#undef X
/*		if(uci_is_fen(ptr->value)){
			LEXER_ASSIGN(TYPE_FEN);
			continue;
		}
		if(uci_is_number(ptr->value)){
			LEXER_ASSIGN(TYPE_NUMBER);
			continue;
		}
		if(uci_is_move(ptr->value)){
			LEXER_ASSIGN(TYPE_MOVE);
			continue;
		}
		LEXER_ASSIGN(SPEC_UNDEF);
	*/
	}

	debug_print("%s", "Finished lexing.");
	return 1;
}
#undef LEXER_ASSIGN

#define PARSER_ABORT(FMT, ...)\
do{\
debug_print(FMT, __VA_ARGS__);\
goto ABORT;\
}while(0)
int uci_parser(UCIToken* tokens){
	UCIToken* node;
	debug_print("%s", "Parsing tokens.");
	for(node = tokens; node; node = node->next){
		debug_print("Reviewing token: %s, %i", node->value, node->id);
		if(TAB_CALLER(node->id) == SPEC_UNDEF)
			goto ABORT;
		switch(node->id){
		/* single word commands */
		case UCI:
		case ISREADY:
		case UCINEWGAME:
		case STOP:
		case PONDERHIT:
		case QUIT:
			uci_clear_table();
			break;
		/* data types */
		case TYPE_STRING:
			if(!node->next){
				TAB_REJECT(TYPE_STRING);
				break;
			}
			if(TAB_CALLER(node->next->id)){
				TAB_REJECT(TYPE_STRING);
				break;
			}
			node->next->id = TYPE_STRING;
			break;
		case TYPE_MOVE:
			if(TAB_CALLER(node->next->id))
				TAB_REJECT(TYPE_MOVE);
			break;
		case TYPE_FEN:
			if(TAB_CALLED(TYPE_FEN, POSITION))
				TAB_ACCEPT(MOVES, POSITION);
			TAB_REJECT(TYPE_FEN);
			break;
		/* complex commands */
		/* DEBUG */
		case DEBUG:
			uci_clear_table();
			TAB_ACCEPT(ON, DEBUG);
			TAB_ACCEPT(OFF, DEBUG);
			break;
		case ON:
			if(TAB_CALLED(ON, DEBUG)){
				TAB_REJECT(ON);
				TAB_REJECT(OFF);
			}else
				goto ABORT;
			break;
		case OFF:
			if(TAB_CALLED(OFF, DEBUG)){
				TAB_REJECT(ON);
				TAB_REJECT(OFF);
			}else
				goto ABORT;
			break;
		/* SETOPTION */
		case SETOPTION:
			uci_clear_table();
			TAB_ACCEPT(NAME, SETOPTION);
			break;
		case NAME:
			if(!node->next)
				goto ABORT;
			if(TAB_CALLED(NAME, SETOPTION))
				TAB_ACCEPT(VALUE, SETOPTION);
			else if(TAB_CALLED(NAME, REGISTER))
				TAB_ACCEPT(CODE, REGISTER);
			else
				goto ABORT;
			TAB_DIRECT(NAME, TYPE_STRING);
			node->next->id = TYPE_STRING;
			break;
		case VALUE:
			if(!node->next)
				goto ABORT;
			TAB_DIRECT(VALUE, TYPE_STRING);
			node->next->id = TYPE_STRING;
			break;
		/* GO */
		case GO:
			uci_clear_table();
			TAB_ACCEPT(SEARCHMOVES, GO);
			TAB_ACCEPT(PONDER,      GO);
			TAB_ACCEPT(WTIME,       GO);
			TAB_ACCEPT(BTIME,       GO);
			TAB_ACCEPT(WINC,        GO);
			TAB_ACCEPT(BINC,        GO);
			TAB_ACCEPT(MOVESTOGO,   GO);
			TAB_ACCEPT(DEPTH,       GO);
			TAB_ACCEPT(NODES,       GO);
			TAB_ACCEPT(MATE,        GO);
			TAB_ACCEPT(MOVETIME,    GO);
			TAB_ACCEPT(INFINITE,    GO);
			break;
		#define CASE_FLAG_NUMBER(ID)\
		case ID:\
			TAB_DIRECT(ID, TYPE_NUMBER);\
			break;
		CASE_FLAG_NUMBER(WTIME);
		CASE_FLAG_NUMBER(BTIME);
		CASE_FLAG_NUMBER(WINC);
		CASE_FLAG_NUMBER(BINC);
		CASE_FLAG_NUMBER(MOVETIME);
		CASE_FLAG_NUMBER(MOVESTOGO);
		CASE_FLAG_NUMBER(NODES);
		CASE_FLAG_NUMBER(MATE);
		#undef CASE_FLAG_NUMBER
		case SEARCHMOVES:
			TAB_DIRECT(SEARCHMOVES, TYPE_MOVE);
			break;
		/* POSITION */
		case POSITION:
			uci_clear_table();
			TAB_ACCEPT(FEN, POSITION);
			TAB_ACCEPT(STARTPOS, POSITION);
			break;
		case STARTPOS:
			TAB_REJECT(FEN);
			TAB_DIRECT(STARTPOS, MOVES);
			break;
		case FEN:
			TAB_REJECT(STARTPOS);
			TAB_DIRECT(FEN, TYPE_FEN);
			break;
		case MOVES:
			TAB_DIRECT(MOVES, TYPE_MOVE);
			break;
		/* REGISTER */
		case REGISTER:
			uci_clear_table();
			TAB_ACCEPT(LATER, REGISTER);
			TAB_ACCEPT(NAME,  REGISTER);
			TAB_ACCEPT(CODE,  REGISTER);
			break;
		case LATER:
			if(TAB_CALLED(LATER, REGISTER)){
				TAB_REJECT(LATER);
				TAB_REJECT(NAME);
				TAB_REJECT(CODE);
			}else
				goto ABORT;
			break;
		case CODE:
			if(TAB_CALLED(CODE, REGISTER)){
				TAB_REJECT(CODE);
				TAB_REJECT(LATER);
			}else
				goto ABORT;
			break;
		/* responses */
		//TODO
		default:
			TAB_REJECT(node->id);
			break;
		}
		if(!node)
			break;
	}
	if(node)
		goto ABORT;
	debug_print("%s", "Finished parsing.");
	return 1;
	ABORT:
	debug_print("%s", "Aborting parsing.");
	return 0;
}

int uci_vectorize(UCIToken* tokens, int* argc, char*** argv){
	int i;
	UCIToken* ptr;
	debug_print("%s", "Vectorizing.");
	ptr = tokens;
	*argc = 0;
	while(ptr){
		++(*argc);
		if(ptr->id != TYPE_STRING){
			ptr = ptr->next;
			continue;
		}
		while(ptr && ptr->id == TYPE_STRING)
			ptr = ptr->next;
	}
	debug_print("Counted %i argument(s).", *argc);
	*argv = uci_alloc(*argc*sizeof(char*));
	if(!*argv){
		*argc = 0;
		goto ABORT;
	}
	ptr = tokens;
	for(i = 0; i < *argc; ++i){
		(*argv)[i] = uci_next_vector(&(ptr));
		if(!(*argv)[i]){
			*argc = i;
			goto ABORT;
		}
	}
	return 1;
	ABORT:
	uci_free_vector(*argc, *argv);
	return 0;
}

