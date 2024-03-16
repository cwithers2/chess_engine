#ifndef UCI_H
#define UCI_H

int uci_parse(char* buffer, int* argc, char*** argv);
int uci_write(char* buffer);

void uci_free_vector(int argc, char** argv);
#endif
