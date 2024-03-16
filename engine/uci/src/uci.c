#include <debug.h>
#include <uci.h>
#include <uci_node.h>
#include <command_parse.h>
#include <command_lex.h>
#include <respond_lex.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

char* uci_next_vector(UCINode** node){
	size_t size, i;
	char* buffer;
	UCINode* ptr;
	debug_print("%s", "Constructing next vector.");
	if((*node)->id != TYPE_SUBSTR){
		debug_print("%s", "Non-stringed node found.");
		size = strlen((*node)->value) + 1;
		buffer = malloc(size*sizeof(char));
		if(!buffer)
			goto ABORT;
		strcpy(buffer, (*node)->value);
		*node = (*node)->next;
		return buffer;
	}
	debug_print("%s", "Stringed node(s) found.");
	size = 0;
	for(ptr = *node; ptr && ptr->id == TYPE_SUBSTR; ptr = ptr->next){
		size += strlen(ptr->value) + 1;
		debug_print("Accumulated discovered size: %li", size);
	}
	buffer = malloc(size*sizeof(char));
	if(!buffer)
		goto ABORT;
	i = 0;
	while(i < size){
		strcpy(buffer+i, (*node)->value);
		i += strlen((*node)->value);
		buffer[i] = ' ';
		++i;
		if(!*node)
			break;
		*node = (*node)->next;
	}
	buffer[size-1] = '\0';
	return buffer;
	ABORT:
	free(buffer);
	return NULL;
}

void uci_vectorize(int* argc, char*** argv){
	int i;
	UCINode* ptr;
	debug_print("%s", "Vectorizing.");
	ptr = uci_node_head.next;
	*argc = 0;
	while(ptr){
		++(*argc);
		if(ptr->id != TYPE_SUBSTR){
			ptr = ptr->next;
			continue;
		}
		while(ptr && ptr->id == TYPE_SUBSTR)
			ptr = ptr->next;
	}
	debug_print("Counted %i argument(s).", *argc);
	*argv = malloc(*argc*sizeof(char*));
	if(!*argv){
		*argc = 0;
		goto ABORT;
	}
	ptr = uci_node_head.next;
	for(i = 0; i < *argc; ++i){
		(*argv)[i] = uci_next_vector(&(ptr));
		if(!(*argv)[i]){
			*argc = i;
			goto ABORT;
		}
	}
	return;
	ABORT:
	fprintf(stderr, "UCI: OUT OF MEMORY\n");
	uci_free_vector(*argc, *argv);
	uci_node_free();
	raise(SIGABRT);
}

int uci_parse(const char* buffer, int* argc, char*** argv){
	int result;
	command_scan_string(buffer);
	debug_print("%s", "Parsing buffer.");
	result = commandparse();
	commandlex_destroy();
	if(result != 0){
		debug_print("%s", "Failed to parse.");
		return 0;
	}
	uci_vectorize(argc, argv);
	uci_node_free();
	debug_print("%s", "Success: uci_parse.");
	return 1;
}

int uci_write(const char* buffer){
	respond_scan_string(buffer);
	if(!respondlex())
		return 0;
	return printf("%s\n", buffer) > 0;
}

void uci_free_vector(const int argc, char** argv){
	int i;
	for(i = 0; i < argc; ++i)
		free(argv[i]);
	free(argv);
}
