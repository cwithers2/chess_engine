#include <stdio.h>
#include <fen_lex.h>
#include <fen_parse.h>
int main(){
	int result;
	char buffer[256];
	fgets(buffer, 256, stdin);
	fen_scan_string(buffer);
	result = fenparse();
	fenlex_destroy();
	return result;
}
