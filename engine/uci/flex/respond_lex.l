%option noyywrap
S                   [ \t\n]+

_rank               [1-8]
_file               [a-h]
_promote            [nbrq]
_coord              {_rank}{_file}
MOVE                {_coord}{2}{_promote}?
MOVES               ({MOVE}{S})*{MOVE}

_digit              [0-9]
NUM                 {_digit}+

_fen1               ([pnbrqkPNBRQK1-8]+\/){7}([pnbrqkPNBRQK1-8]+)
_fen2               [wb]
_fen3               ([kqKQ]{1,4})|-
_fen4               ([a-h][36])|-
FEN                 {_fen1}{S}{_fen2}{S}{_fen3}{S}{_fen4}{S}{NUM}{S}{NUM}

SUBSTR              [^ \t\n]+
X                   ({SUBSTR}{S})*{SUBSTR}

ID                  id{S}(name{S}{X}|author{S}{X})
UCIOK               uciok
READYOK             readyok
BESTMOVE            bestmove{S}{MOVE}(ponder{S}{MOVE})?
COPYPROTECTION      copyprotection{S}(checking|ok|error)
REGISTRATION        registration{S}(checking|ok|error)
INFO                info(({S}string{S}{X})|(({S}depth{S}{NUM})?({S}seldepth{S}{NUM})?({S}time{S}{NUM})?({S}nodes{S}{NUM})?({S}pv{S}{MOVES})?({S}multipv{S}{NUM})?({S}score{S}(cp{S}{NUM})|(mate{S}{NUM})({S}upperbound)?({S}lowerbound)?)?({S}currmove{S}{NUM})?({S}hashfull{S}{NUM})?({S}nps{S}{NUM})?({S}tbhits{S}{NUM})?({S}sbhits{S}{NUM})?({S}cpuload{S}{NUM})?({S}refutation{S}{MOVES})?({S}currline{S}({NUM})?{S}{MOVES})?))
OPTION              option{S}name{S}{X}{S}type{S}(check|spin|combo|button|string)(default{S}{X})?(min{S}{X})?(max{S}{X})?(var{S}{X})?

MATCH_VALID         ^({ID}|{UCIOK}|{READYOK}|{BESTMOVE}|{COPYPROTECTION}|{REGISTRATION}|{INFO}|{OPTION}){S}
MATCH_ANY           .*\n
%%
{MATCH_VALID}       return 1;
{MATCH_ANY}         return 0;
