#ifndef UCI_H
#define UCI_H

typedef enum UCI_PARSE_MODE UCI_PARSE_MODE;

enum UCI_PARSE_MODE{
	UCI_PARSE_INPUT,
	UCI_PARSE_OUTPUT
};

int uci_init();
void uci_destroy();

int uci_write(char* fmt, ...);
int uci_parse(char* buffer, int* argc, char*** argv, UCI_PARSE_MODE mode);

void uci_free_vector(int argc, char** argv);
#endif
