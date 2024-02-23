#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include <debug.h>
#include <getline_noblock.h>
#include <uci.h>
#include <commands.h>
#include <responses.h>

/** DEFINITIONS :) **/

#ifndef NO_DEBUG
#define UCI_THREAD_COOLDOWN_S 2
#else
#define UCI_THREAD_COOLDOWN_S 0
#endif

#define _whitespace ' ':case '\t':case '\n'
#define _is_keyword(kw, val) (strcmp(kw, val) == 0)

typedef struct Tokens Tokens;
typedef struct QueueNode QueueNode;

/** DATA TYPES :) **/
struct QueueNode{
	QueueNode* next;
	char** argv;
	int argc;
	int garbage;
};

struct UCIObject{
	int exit;
	thrd_t thread;
	QueueNode head;
	QueueNode* last;
	mtx_t modify;
};

enum GarbageStatus{
	GARBAGE_WAITING = 0,
	GARBAGE_IN_USE,
	GARBAGE_EXPIRED
};

/** PRIVATE FORWARD DECLARATIONS :) **/
int work_thread(void* arg);
void push(UCIObject* uci, QueueNode* node);
QueueNode* pop(UCIObject* uci);
int valid_command(char* value);
int valid_response(char* value);
void free_vector(char** argv, int argc);
void free_node(QueueNode* node);
void cooldown();
QueueNode* next_clean_node(UCIObject* uci);
int is_whitespace(const char ch);
void lock(UCIObject* uci);
void unlock(UCIObject* uci);
QueueNode* create_node(char* buffer);
void collect_garbage(UCIObject* uci);

/** PUBILC LIBRARY FUNCTIONS :) **/
UCIObject* uci_init(){
	int result;
	UCIObject* uci;
	getline_noblock_init();
	uci = (UCIObject*)calloc(1, sizeof(UCIObject)); /** CALLOC **/
	if (!uci)
		goto ERROR_ALLOC;
	result = mtx_init(&(uci->modify), mtx_plain|mtx_recursive);
	if(result != thrd_success)
		goto ERROR_MUTEX;

	result = thrd_create(&(uci->thread), work_thread, uci);
	if(result != thrd_success)
		goto ERROR_THREAD;

	return uci;

	ERROR_THREAD:
	ERROR_MUTEX:
	free(uci);
	ERROR_ALLOC:
	return NULL;
}

void uci_destroy(UCIObject* uci){
	QueueNode* node;
	QueueNode* garbage;
	uci->exit = 1;
	thrd_join(uci->thread, NULL);
	node = uci->head.next;
	while(node){
		garbage = node;
		node = node->next;
		free_node(garbage);
	}
	mtx_destroy(&(uci->modify));
	free(uci);
}

int uci_respond(UCIObject* uci, char* id, char** argv, int argc){
	int i;
	if(!valid_response(id))
		return 1;
	printf("%s", id);
	for(i = 0; i < argc; ++i)
		printf(" %s", argv[i]);
	printf("\n");
	return 0;
}

int uci_poll(UCIObject* uci, char*** argv, int* argc){
	QueueNode* node;
	node = next_clean_node(uci);
	if(!node)
		return 0;
	*argv = node->argv;
	*argc = node->argc;
	return 1;
}

/** PRIVATE LIBRARY FUNCTIONS :) **/
void collect_garbage(UCIObject* uci){
	QueueNode* node;
	QueueNode* garbage;
	debug_print("%s\n", "Looking for garbage.");
	node = uci->head.next;
	if(!node)
		return;
	while(node && node->garbage == GARBAGE_EXPIRED){
		debug_print("%s\n", "Collecting garbage node.");
		garbage = node;
		node = pop(uci);
		free_node(garbage);
	}
}

QueueNode* next_clean_node(UCIObject* uci){
	QueueNode* node;
	debug_print("%s\n", "Looking for next clean node in queue.");
	node = uci->head.next;
	while(node){
		if(!node->garbage){
			debug_print("%s\n", "Clean node found.");
			node->garbage = GARBAGE_IN_USE;
			break;
		}
		if(node->garbage == GARBAGE_IN_USE)
			node->garbage = GARBAGE_EXPIRED;
		node = node->next;
	}
	return node;
}

void lock(UCIObject* uci){
	debug_print("%s\n", "Locking UCIObject.");
	if(mtx_lock(&(uci->modify)) == thrd_success)
		return;
	debug_print("%s\n", "Failed to lock UCIObject. Aborting.");
	uci_destroy(uci);
	abort();
}

void unlock(UCIObject* uci){
	debug_print("%s\n", "Unlocking UCIObject.");
	if(mtx_unlock(&(uci->modify)) == thrd_success)
		return;
	debug_print("%s\n", "Failed to unlock UCIObject. Aborting.");
	uci_destroy(uci);
	abort();
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

int work_thread(void* arg){
	char buffer[UCI_INPUT_BUFFER_LEN];
	UCIObject* uci;
	QueueNode* node;
	uci = (UCIObject*)arg;
	debug_print("%s\n", "Starting main loop.");
	while(!uci->exit){
		if(!getline_noblock(buffer, UCI_INPUT_BUFFER_LEN)){
			debug_print("%s\n", "No input detected.");
			collect_garbage(uci);
			cooldown();
			continue;
		}
		debug_print("%s\n", "Input detected.");
		node = create_node(buffer);
		if(!node){
			continue;
		}
		push(uci, node);
		debug_print("%s\n", "Input processed successfully.");
	}
	return 0;
}

void push(UCIObject* uci, QueueNode* node){
	lock(uci);
	if(!uci->head.next)
		uci->head.next = node;
	else
		uci->last->next = node;
	uci->last = node;
	unlock(uci);
}

QueueNode* pop(UCIObject* uci){
	QueueNode* node;
	debug_print("%s\n", "Popping node from queue.");
	lock(uci);
	if(!uci->head.next)
		return NULL;
	node = uci->head.next;
	uci->head.next = node->next;
	unlock(uci);
	return node;
}

void cooldown(){
	static struct timespec wait = {
		.tv_sec=UCI_THREAD_COOLDOWN_S,
		.tv_nsec=UCI_THREAD_COOLDOWN_NS
	};
	debug_print("%s\n", "Performing cooldown.");
	thrd_sleep(&wait, NULL);
}

void free_vector(char** argv, int argc){
	int i;
	debug_print("%s\n", "Freeing vector.");
	for(i = 0; i < argc; ++i){
		free(argv[i]);
	}
	free(argv);
}

void free_node(QueueNode* node){
	debug_print("%s\n", "Freeing node.");
	free_vector(node->argv, node->argc);
	free(node);
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

int copy_tokens(QueueNode* node, char* buffer){
	int reading;
	int index;
	int length;
	char* ptr;
	char* copy;
	reading = 0;
	ptr = buffer;
	index = 0;
	length = 2;
	debug_print("%s\n", "Copying tokens to node.");
	while(index < node->argc){
		if(is_whitespace(*ptr)){
			if(reading){
				debug_print("Allocating buffer for token %d.\n", index+1);
				node->argv[index] = (char*)calloc(length, sizeof(char));
				if(!node->argv[index]){
					debug_print("%s\n", "Failed to allocate.");
					node->argc = index;
					return 0;
				}
				memcpy(node->argv[index], copy, length * sizeof(char));
				debug_print("%s\n", "Token copied.");
				++index;
				length = 2;
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

QueueNode* create_node(char* buffer){
	int n;
	QueueNode* node;
	debug_print("%s\n", "Allocating node.");
	node = (QueueNode*)calloc(1, sizeof(QueueNode)); /** CALLOC **/
	if(!node){
		debug_print("%s\n", "Failed to allocate.");
		goto ERROR;
	}
	n = count_tokens(buffer);
	if(!n)
		goto ERROR;
	debug_print("%s\n", "Allocating node vector.");
	node->argv = (char**)malloc(n*sizeof(char*));
	if(!node->argv){
		debug_print("%s\n", "Failed to allocate.");
		goto ERROR;
	}
	node->argc = n;
	if(!copy_tokens(node, buffer))
		goto ERROR;
	if(!valid_command(node->argv[0]))
		goto ERROR;
	return node;
	ERROR:
	debug_print("%s\n", "Failed to create node.");
	free_node(node);
	return NULL;
}

#undef _whitespace
#undef _is_keyword
