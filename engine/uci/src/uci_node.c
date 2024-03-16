#include <uci_node.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

UCINode  uci_node_head = {.next=NULL};
UCINode* uci_node_tail = &uci_node_head;

void uci_node_push_or_die(const char* value, int id){
	UCINode* node;
	node = (UCINode*)malloc(sizeof(UCINode*));
	if(!node)
		goto ABORT;
	node->value = strdup(value);
	if(!node->value)
		goto ABORT;
	node->id = id;
	uci_node_tail->next = node;
	uci_node_tail = node;
	uci_node_tail->next = NULL;
	return;
	ABORT:
	fprintf(stderr, "UCI: OUT OF MEMORY\n");
	free(node);
	uci_node_free();
	raise(SIGABRT);
}

void uci_node_free(){
	UCINode* garbage;
	UCINode* head = uci_node_head.next;
	while(head){
		garbage = head;
		head = head->next;
		free(garbage->value);
		free(garbage);
	}
	uci_node_tail = &uci_node_head;
	uci_node_head.next = NULL;
}
