#ifndef TOKENIZE_H
#define TOKENIZE_H

int count_until_token(char* buffer, char* token, char* delim);
int check_token(char* buffer, char* token, char* delim);
int count_while_delim(char* buffer, char* delim);
int count_until_delim(char* buffer, char* delim);

//depreciated
int get_token(char* buffer, char** token_start, char* delim);

#endif
