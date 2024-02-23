#ifndef UCI_H
#define UCI_H

/** DEFINITIONS :) **/
#ifndef UCI_INPUT_BUFFER_LEN
#define UCI_INPUT_BUFFER_LEN 256
#endif
#ifndef UCI_THREAD_COOLDOWN_NS
#define UCI_THREAD_COOLDOWN_NS 250000000
#endif

typedef struct UCIObject UCIObject;

/** FORWARD DECLARATIONS :) **/
UCIObject* uci_init();
void uci_destroy(UCIObject* uci);

int uci_poll(UCIObject* uci, char*** argv, int* argc);
int uci_respond(UCIObject* uci, char* id, char** argv, int argc);


#endif
