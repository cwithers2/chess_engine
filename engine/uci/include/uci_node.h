#ifndef UCI_NODE_H
#define UCI_NODE_H

typedef struct UCINode UCINode;

struct UCINode{
	UCINode* next;
	char* value;
	int id;
};

extern UCINode  uci_node_head;
extern UCINode* uci_node_tail;

void uci_node_push_or_die(const char* value, int id);
void uci_node_free();

#endif
