%{
#include <stdio.h>
extern char *fentext;
int fenlex(void);
int fenerror (char const *s){
	fprintf (stderr, "%s: %s\n", s, fentext);
}
%}
%token TOKEN_PIECE
%token TOKEN_SLASH
%token TOKEN_PLAYER
%token TOKEN_CASTLE
%token TOKEN_NO_CASTLE
%token TOKEN_EN_PASSANT
%token TOKEN_NO_EN_PASSANT
%token TOKEN_NUMBER
%%
fen:
	field_1 field_2 field_3 field_4 field_5 field_6
	;
field_1:
	pieces
	;
pieces:
	row TOKEN_SLASH row TOKEN_SLASH row TOKEN_SLASH row TOKEN_SLASH row TOKEN_SLASH row TOKEN_SLASH row TOKEN_SLASH row
	;
row:
	TOKEN_PIECE TOKEN_PIECE TOKEN_PIECE TOKEN_PIECE TOKEN_PIECE TOKEN_PIECE TOKEN_PIECE TOKEN_PIECE
	;
field_2:
	TOKEN_PLAYER
	;
field_3:
	TOKEN_NO_CASTLE
	| castles
	;
castles:
	TOKEN_CASTLE
	|TOKEN_CASTLE TOKEN_CASTLE
	|TOKEN_CASTLE TOKEN_CASTLE TOKEN_CASTLE
	|TOKEN_CASTLE TOKEN_CASTLE TOKEN_CASTLE TOKEN_CASTLE
	;
field_4:
	TOKEN_NO_EN_PASSANT
	|TOKEN_EN_PASSANT
	;
field_5:
	TOKEN_NUMBER
	;
field_6:
	TOKEN_NUMBER
	;
