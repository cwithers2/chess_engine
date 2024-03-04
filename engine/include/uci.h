#ifndef UCI_H
#define UCI_H

#ifndef UCI_INPUT_BUFFER_LEN
#define UCI_INPUT_BUFFER_LEN 256
#endif

int uci_init();
void uci_destroy();

int uci_poll(int* argc, char*** argv);
int uci_respond(int argc, char** argv);

#endif
