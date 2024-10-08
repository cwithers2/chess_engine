#ifndef DEBUG
#define DEBUG
#include <debug.h>
#undef  DEBUG
#else
#include <debug.h>
#endif

void debug_print_bitboard(u64 bboard){
	char buffer[64+9];
	int row, col, index = 0;
	u64 focus = 0x8000000000000000;
	for(row = 0; row < 8; ++row){
		for(col = 0; col < 8; ++col){
			buffer[index++] = focus & bboard? 'x' : '.';
			focus >>= 1;
		}
		buffer[index++] = '\n';
	}
	buffer[index-1] = '\0';
	debug_print("%s", buffer);
}

static void print_target(Board* board){
	char str[3] = {0};
	if(board->target)
		board_format_pos(board->target, str);
	debug_print("TARGET : %2s", str);
}

static void print_castle(Board* board){
	char str[5] = {0};
	int index = 0;
	#define X(SIDE, CHAR, POS)\
	if(board->castle[SIDE] & POS)\
		str[index++] = CHAR;
	#include <x/castle_extended.h>
	#undef X
	debug_print("CASTLE : %4s", str);
}

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
	if(board->active == SIDE)\
		debug_print("%s", "ACTIVE : " #SIDE);
	#include <x/side.h>
	#undef X
	print_castle(board);
	print_target(board);
	debug_print("HALFMV : %u", board->halfmoves);
	debug_print("FULLMV : %u", board->fullmoves);
}
