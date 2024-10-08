%option noyywrap
%{
#include <uci_node.h>

#define LEX(X)\
do{\
	uci_node_push_or_die(yytext, X);\
	return X;\
}while(0)

#include <command_parse.h>
#include <fen_parse.h>
int fen_scan_string(const char*);
void fenlex_destroy();
#define FLAG(X)       flags[X-SPEC_FIRST+1]  = 1
#define UNFLAG(X)     flags[X-SPEC_FIRST+1]  = 0
#define FLAGGED(X)   (flags[X-SPEC_FIRST+1] != 0)
#define FLAG_SIZE    (SPEC_LAST-SPEC_FIRST)-1

#define RESOLVE_STRING(X)\
if(!FLAGGED(X))\
	LEX(TYPE_SUBSTR);
%}

%s STATE_UCI
%s STATE_DEBUG
%s STATE_ISREADY
%s STATE_SETOPTION
%s STATE_REGISTER
%s STATE_UCINEWGAME
%s STATE_POSITION
%s STATE_GO
%s STATE_STOP
%s STATE_PONDERHIT
%s STATE_QUIT

S                   [ \t\n]+

_rank               [1-8]
_file               [a-h]
_promote            [nbrq]
_coord              {_file}{_rank}
MOVE                {_coord}{2}{_promote}?

_digit              [0-9]
NUM                 {_digit}+

_fen1               ([pnbrqkPNBRQK1-8]+\/){7}([pnbrqkPNBRQK1-8]+)
_fen2               [wb]
_fen3               ([kqKQ]{1,4})|([a-hA-H]{1,4})|-
_fen4               ([a-h][36])|-
FEN                 {_fen1}{S}{_fen2}{S}{_fen3}{S}{_fen4}{S}{NUM}{S}{NUM}

SUBSTR              [^ \t\n]+
%%
	static unsigned char flags[FLAG_SIZE] = {0};

<*>{S}
<*>{MOVE} {
	RESOLVE_STRING(TYPE_MOVE);
	LEX(TYPE_MOVE);
}
<*>{NUM} {
	RESOLVE_STRING(TYPE_NUM);
	LEX(TYPE_NUM);
}
<*>{FEN} {
	RESOLVE_STRING(TYPE_FEN);
	//token is not a string if the control reaches this point.
	int result;
	fen_scan_string(commandtext);
	result = fenparse();
	if(result){
		//error.
		fenlex_destroy();
		return commandtext[0];
	}
	LEX(TYPE_FEN);
}

<INITIAL>uci {
	BEGIN(STATE_UCI);
	LEX(TOKEN_UCI);
}


<INITIAL>debug {
	BEGIN(STATE_DEBUG);
	FLAG(TOKEN_ON);
	FLAG(TOKEN_OFF);
	LEX(TOKEN_DEBUG);
}
<STATE_DEBUG>on {
	RESOLVE_STRING(TOKEN_ON);
	UNFLAG(TOKEN_ON);
	UNFLAG(TOKEN_OFF);
	LEX(TOKEN_ON);
}
<STATE_DEBUG>off {
	RESOLVE_STRING(TOKEN_OFF);
	UNFLAG(TOKEN_ON);
	UNFLAG(TOKEN_OFF);
	LEX(TOKEN_OFF);
}


<INITIAL>isready {
	BEGIN(STATE_ISREADY);
	LEX(TOKEN_ISREADY);
}


<INITIAL>setoption {
	BEGIN(STATE_SETOPTION);
	FLAG(TOKEN_NAME);
	FLAG(TYPE_SUBSTR);
	LEX(TOKEN_SETOPTION);
}
<STATE_SETOPTION>name {
	RESOLVE_STRING(TOKEN_NAME);
	UNFLAG(TOKEN_NAME);
	FLAG(TOKEN_VALUE);
	LEX(TOKEN_NAME);
}
<STATE_SETOPTION>value {
	RESOLVE_STRING(TOKEN_VALUE);
	UNFLAG(TOKEN_VALUE);
	LEX(TOKEN_VALUE);
}


<INITIAL>register {
	BEGIN(STATE_REGISTER);
	FLAG(TOKEN_LATER);
	FLAG(TOKEN_NAME);
	LEX(TOKEN_REGISTER);
}
<STATE_REGISTER>later {
	RESOLVE_STRING(TOKEN_LATER);
	UNFLAG(TOKEN_LATER);
	UNFLAG(TOKEN_NAME);
	LEX(TOKEN_LATER);
}
<STATE_REGISTER>name {
	RESOLVE_STRING(TOKEN_NAME);
	UNFLAG(TOKEN_LATER);
	UNFLAG(TOKEN_NAME);
	FLAG(TOKEN_CODE);
	LEX(TOKEN_NAME);
}
<STATE_REGISTER>code {
	RESOLVE_STRING(TOKEN_CODE);
	UNFLAG(TOKEN_CODE);
	LEX(TOKEN_CODE);
}


<INITIAL>ucinewgame {
	BEGIN(STATE_UCINEWGAME);
	LEX(TOKEN_UCINEWGAME);
}

<INITIAL>position {
	BEGIN(STATE_POSITION);
	FLAG(TOKEN_FEN);
	FLAG(TOKEN_STARTPOS);
	FLAG(TOKEN_MOVES);
	FLAG(TYPE_FEN);
	FLAG(TYPE_MOVE);
	LEX(TOKEN_POSITION);
}
<STATE_POSITION>fen {
	RESOLVE_STRING(TOKEN_FEN);
	UNFLAG(TOKEN_FEN);
	UNFLAG(TOKEN_STARTPOS);
	LEX(TOKEN_FEN);
}

<STATE_POSITION>startpos {
	RESOLVE_STRING(TOKEN_STARTPOS);
	UNFLAG(TOKEN_FEN);
	UNFLAG(TOKEN_STARTPOS);
	LEX(TOKEN_STARTPOS);
}
<STATE_POSITION>moves {
	RESOLVE_STRING(TOKEN_MOVES);
	UNFLAG(TOKEN_FEN);
	UNFLAG(TOKEN_STARTPOS);
	UNFLAG(TOKEN_MOVES);
	LEX(TOKEN_MOVES);
}



<INITIAL>go {
	BEGIN(STATE_GO);
	FLAG(TOKEN_SEARCHMOVES);
	FLAG(TOKEN_PONDER);
	FLAG(TOKEN_WTIME);
	FLAG(TOKEN_BTIME);
	FLAG(TOKEN_WINC);
	FLAG(TOKEN_BINC);
	FLAG(TOKEN_MOVESTOGO);
	FLAG(TOKEN_DEPTH);
	FLAG(TOKEN_NODES);
	FLAG(TOKEN_MATE);
	FLAG(TOKEN_MOVETIME);
	FLAG(TOKEN_INFINITE);
	FLAG(TYPE_MOVE);
	FLAG(TYPE_NUM);
	LEX(TOKEN_GO);
}

<STATE_GO>searchmoves {
	RESOLVE_STRING(TOKEN_SEARCHMOVES);
	UNFLAG(TOKEN_SEARCHMOVES);
	LEX(TOKEN_SEARCHMOVES);
}
<STATE_GO>ponder {
	RESOLVE_STRING(TOKEN_PONDER);
	UNFLAG(TOKEN_PONDER);
	LEX(TOKEN_PONDER);
}
<STATE_GO>wtime {
	RESOLVE_STRING(TOKEN_WTIME);
	UNFLAG(TOKEN_WTIME);
	LEX(TOKEN_WTIME);
}
<STATE_GO>btime {
	RESOLVE_STRING(TOKEN_BTIME);
	UNFLAG(TOKEN_BTIME);
	LEX(TOKEN_BTIME);
}
<STATE_GO>winc {
	RESOLVE_STRING(TOKEN_WINC);
	UNFLAG(TOKEN_WINC);
	LEX(TOKEN_WINC);
}
<STATE_GO>binc {
	RESOLVE_STRING(TOKEN_BINC);
	UNFLAG(TOKEN_BINC);
	LEX(TOKEN_BINC);
}
<STATE_GO>movestogo {
	RESOLVE_STRING(TOKEN_MOVESTOGO);
	UNFLAG(TOKEN_MOVESTOGO);
	LEX(TOKEN_MOVESTOGO);
}
<STATE_GO>depth {
	RESOLVE_STRING(TOKEN_DEPTH);
	UNFLAG(TOKEN_DEPTH);
	LEX(TOKEN_DEPTH);
}
<STATE_GO>nodes {
	RESOLVE_STRING(TOKEN_NODES);
	UNFLAG(TOKEN_NODES);
	LEX(TOKEN_NODES);
}
<STATE_GO>mate {
	RESOLVE_STRING(TOKEN_MATE);
	UNFLAG(TOKEN_MATE);
	LEX(TOKEN_MATE);
}
<STATE_GO>movetime {
	RESOLVE_STRING(TOKEN_MOVETIME);
	UNFLAG(TOKEN_MOVETIME);
	LEX(TOKEN_MOVETIME);
}
<STATE_GO>infinite {
	RESOLVE_STRING(TOKEN_INFINITE);
	UNFLAG(TOKEN_INFINITE);
	LEX(TOKEN_INFINITE);
}


<INITIAL>stop {
	BEGIN(STATE_STOP);
	LEX(TOKEN_STOP);
}


<INITIAL>ponderhit {
	BEGIN(STATE_PONDERHIT);
	LEX(TOKEN_PONDERHIT);
}


<INITIAL>quit {
	BEGIN(STATE_QUIT);
	LEX(TOKEN_QUIT);
}

<*>{SUBSTR} {
	LEX(TYPE_SUBSTR);
}
