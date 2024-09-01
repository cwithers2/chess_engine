%option noyywrap
%{
#include <stdlib.h>
#include <fen_parse.h>
%}
%s FIELD_2
%s FIELD_3
%s FIELD_4
%s FIELD_5
%s FIELD_6
%s FIELD_ERROR
PIECE      [pnbrqkPNBRQK]
PLAYER     [\-wb]
CASTLE     [kqKQ]
EN_PASSANT [a-h][36]
SLASH      \/
DASH       \-
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
<INITIAL>{NUMBER} {
	repeat = atoi(fentext)-1;
	return TOKEN_PIECE;
}
<INITIAL>{SLASH} {
	return TOKEN_SLASH;
}
<FIELD_2>{PLAYER} {
	return TOKEN_PLAYER;
}
<FIELD_3>{CASTLE} {
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
