%{
#include <stdio.h>
extern char *commandtext;
int commandlex(void);
int commanderror (char const *s){
	fprintf (stderr, "%s: %s\n", s, commandtext);
}
%}
%token SPEC_FIRST

%token TYPE_SUBSTR TYPE_MOVE TYPE_NUM TYPE_FEN

%token TOKEN_UCI
%token TOKEN_DEBUG
%token TOKEN_ISREADY
%token TOKEN_SETOPTION
%token TOKEN_REGISTER
%token TOKEN_UCINEWGAME
%token TOKEN_POSITION
%token TOKEN_GO
%token TOKEN_STOP
%token TOKEN_PONDERHIT
%token TOKEN_QUIT

%token TOKEN_ON
%token TOKEN_OFF
%token TOKEN_NAME
%token TOKEN_VALUE
%token TOKEN_LATER
%token TOKEN_CODE
%token TOKEN_FEN
%token TOKEN_STARTPOS
%token TOKEN_MOVES
%token TOKEN_SEARCHMOVES
%token TOKEN_PONDER
%token TOKEN_WTIME
%token TOKEN_BTIME
%token TOKEN_WINC
%token TOKEN_BINC
%token TOKEN_MOVESTOGO
%token TOKEN_DEPTH
%token TOKEN_NODES
%token TOKEN_MATE
%token TOKEN_MOVETIME
%token TOKEN_INFINITE

%token SPEC_LAST
%%
commands:
	uci
	| debug
	| isready
	| setoption
	| register
	| ucinewgame
	| position
	| go
	| stop
	| ponderhit
	| quit
	;
uci:
	TOKEN_UCI
	;
debug:
	TOKEN_DEBUG TOKEN_ON
	| TOKEN_DEBUG TOKEN_OFF
	;
isready:
	TOKEN_ISREADY
	;
setoption:
	TOKEN_SETOPTION TOKEN_NAME string
	| TOKEN_SETOPTION TOKEN_NAME string TOKEN_VALUE string
	;
register:
	TOKEN_REGISTER TOKEN_LATER
	| TOKEN_REGISTER TOKEN_NAME string TOKEN_CODE string
	;
ucinewgame:
	TOKEN_UCINEWGAME
	;
position:
	TOKEN_POSITION TOKEN_MOVES moves
	| TOKEN_POSITION TOKEN_FEN TYPE_FEN TOKEN_MOVES moves
	| TOKEN_POSITION TOKEN_STARTPOS TOKEN_MOVES moves
	;
go:
	TOKEN_GO
	| TOKEN_GO go_options
	;
go_options:
	go_option
	| go_options go_option
	;
go_option:
	TOKEN_SEARCHMOVES  moves
	| TOKEN_PONDER
	| TOKEN_WTIME      number
	| TOKEN_BTIME      number
	| TOKEN_WINC       number
	| TOKEN_BINC       number
	| TOKEN_MOVESTOGO  number
	| TOKEN_DEPTH      number
	| TOKEN_NODES      number
	| TOKEN_MATE       number
	| TOKEN_MOVETIME   number
	| TOKEN_INFINITE
	;
stop:
	TOKEN_STOP
	;
ponderhit:
	TOKEN_PONDERHIT
	;
quit:
	TOKEN_QUIT
	;
number:
	TYPE_NUM
	;
move:
	TYPE_MOVE
	;
moves:
	move
	| moves move
	;
string:
	TYPE_SUBSTR
	| string TYPE_SUBSTR
	;
