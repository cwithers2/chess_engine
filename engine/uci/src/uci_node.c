#include <uci_node.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

UCINode  uci_head;
UCINode* uci_tail;

void uci_node_push_or_die(const char* value, int id){
	UCINode* node;
	node = (UCINode*)malloc(sizeof(UCINode*));
	if(!node)
		goto ABORT;
	node->value = strdup(value);
	if(!node->value)
		goto ABORT;
	node->id = id;
	uci_tail->next = node;
	uci_tail = node;
	uci_tail->next = NULL;
	return;
	ABORT:
	fprintf(stderr, "UCI: OUT OF MEMORY\n");
	free(node);
	raise(SIGABRT);
}

void uci_node_free(){
	UCINode* garbage;
	UCINode* head = uci_head.next;
	while(head){
		garbage = head;
		head = head->next;
		free(garbage->next);
		free(garbage);
	}
	uci_tail = &uci_head;
	uci_head.next = NULL;
}
