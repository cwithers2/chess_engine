%option noyywrap
%{
#include <stdlib.h>
#include <fen_parse.h>

char castle_tally[2];
char castle_cache[2];
int  castle_index;
#include <ctype.h>
#define UPDATE_CASTLE(C) \
do{\
	castle_index = islower(C)?1:0;\
	if(castle_cache[castle_index] == C)\
		return C;\
	castle_cache[castle_index] = C;\
	if(castle_tally[castle_index] >= 2)\
		return C;\
	++castle_tally[castle_index];\
}while(0)
#define RESET_CASTLE() \
do{\
	memset(castle_tally,0,2);\
	memset(castle_cache,0,2);\
}while(0)
%}
%s FIELD_2
%s FIELD_3
%s FIELD_4
%s FIELD_5
%s FIELD_6
%s FIELD_ERROR
PIECE      [pnbrqkPNBRQK]
PLAYER     [\-wb]
CASTLE     [kqKQ]|[a-h]|[A-H]
EN_PASSANT [a-h][36]
SLASH      \/
DASH       \-
EMPTY      [1-8]
DIGIT      [0-9]
NUMBER     {DIGIT}+
SPACE      [ \t]+
%%
	static int repeat = 0;
	if(repeat){
		--repeat;
		return TOKEN_PIECE;
	}
<*>{SPACE} {
	BEGIN(YYSTATE+1);
}
<INITIAL>{PIECE} {
	return TOKEN_PIECE;
}
<INITIAL>{EMPTY} {
	repeat = atoi(fentext)-1;
	return TOKEN_PIECE;
}
<INITIAL>{SLASH} {
	return TOKEN_SLASH;
}
<FIELD_2>{PLAYER} {
	RESET_CASTLE();
	return TOKEN_PLAYER;
}
<FIELD_3>{CASTLE} {
	UPDATE_CASTLE(fentext[0]);
	return TOKEN_CASTLE;
}
<FIELD_3>{DASH} {
	return TOKEN_NO_CASTLE;
}
<FIELD_4>{EN_PASSANT} {
	return TOKEN_EN_PASSANT;
}
<FIELD_4>{DASH} {
	return TOKEN_NO_EN_PASSANT;
}
<FIELD_5>{NUMBER} {
	return TOKEN_NUMBER;
}
<FIELD_6>{NUMBER} {
	return TOKEN_NUMBER;
}
<FIELD_ERROR>.+ {
	return fentext[0];
}
