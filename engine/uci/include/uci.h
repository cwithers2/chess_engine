#ifndef UCI_H
#define UCI_H

/** int uci_parse(...)
	Parse a message from the UI.

	Parameters
	----------
	const char* message
		The message from the UI.
	int* argc
		The number of tokens generated from the message.
	char*** argv
		The tokens generated from the message.

	Return
	------
	int
		0: Failed to parse the message.
		1: Successfully parsed the message.

	Note
	----
	This function will raise SIGABRT if there is a memory shortage.

	Note
	----
	The dereferenced argv parameter will need to be freed on success.
**/
int uci_parse(const char* message, int* argc, char*** argv);



/** uci_write(...)
	Write a message to the UI.

	Parameters
	----------
	const char* message
		The message to the UI.

	Return
	------
	int
		0: The message was not valid; The message was not sent.
		1: The message was sent.
**/
int uci_write(const char* message);



void uci_free_vector(const int argc, char** argv);
#endif
