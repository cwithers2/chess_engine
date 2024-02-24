#ifndef UCI_H
#define UCI_H

/** DEFINITIONS :) **/

#ifndef UCI_INPUT_BUFFER_LEN
#define UCI_INPUT_BUFFER_LEN 256
#endif

/** FORWARD DECLARATIONS :) **/

/* Function: uci_init
 * ------------------
 * Initialize UCI data.
 * NOTE: Call before using any other UCI functions.
 */
int uci_init();

/* Function: uci_destroy
 * ---------------------
 * Destroy UCI data.
 * NOTE: Call after using all other UCI functions.
 */
void uci_destroy();

/* Function: uci_poll
 * ------------------
 * Gets input tokens from the user interface.
 * NOTE: does not block.
 *
 * Parameters
 * ----------
 * argc : int*
 *    Returns the number of tokens sent by the UI.
 * argv : char***
 *    Returns the tokens sent by the UI.
 * 
 * Returns
 * -------
 * int:
 *    0: No input is available from the UI.
 *    1: Tokens are ready.
 */
int uci_poll(int* argc, char*** argv);

/* Function: uci_respond
 * ---------------------
 * Send response token to the user interface.
 *
 * Parameters
 * ----------
 * id : char*
 *    A valid response token for UCI communication ie "info".
 * argc : int
 *    The number of tokens being sent to the UI.
 * argv : char**
 *    The tokens to send to the UI.
 *
 * Returns
 * -------
 * int:
 *    0: Argument:id was not valid; No input was sent.
 *    1: Tokens were sent successfully.
 */
int uci_respond(char* id, int argc, char** argv);

#endif
