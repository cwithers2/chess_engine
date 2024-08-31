#ifndef DEBUG_H
#define DEBUG_H

//set flag NO_DEBUG to remove debug tokens from compliation.
#ifdef NO_DEBUG
#define debug_print(fmt, ...) ;
#define debug_print_board(B)  ;
#else
#include <stdio.h>
#define debug_print(fmt, ...) \
do{fprintf(stderr, fmt "\n", __VA_ARGS__); }while(0)

void debug_print_board(Board* board){
	char buffer[64+9];
	int row, col, index = 0;
	u64 focus = 0x8000000000000000;
	debug_print("%s", "Printing board data:");
	for(row = 0; row < 8; ++row){
		for(col = 0; col < 8; ++col){
			#define X(SIDE, PIECE, CHAR)\
			if(focus & board->pieces[SIDE][PIECE]){\
				buffer[index++] = CHAR;\
				focus >>= 1;\
				continue;\
			}
			#include <x/white.h>
			#include <x/black.h>
			#undef X
			buffer[index++] = '.';
			focus >>= 1;
		}
		buffer[index++] = '\n';
	}
	buffer[index-1] = '\0';
	debug_print("%s", buffer);
	#define X(SIDE, CHAR)\
	if(board->active == CHAR)\
		debug_print("%s", "ACTIVE : " #SIDE);
	#include <x/side.h>
	#undef X
	debug_print(
		"CASTLE : %c%c%c%c",
		board->castle[WHITE][0],
		board->castle[WHITE][1],
		board->castle[BLACK][0],
		board->castle[BLACK][1]);
	debug_print("TARGET : %2s", board->target);
	debug_print("HALFMV : %u", board->halfmoves);
	debug_print("FULLMV : %u", board->fullmoves);
}
#endif

#endif
