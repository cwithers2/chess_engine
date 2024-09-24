#include <stdio.h>
#include "include/board.h"
#include "include/magic.h"
#include "include/debug.h"

int main(int argc, char **argv){
	if(!magic_init())
		return 1;
	u64 piece  = 0x0000000000200000;
	u64 block  = 0x2020000000FF0000;
	u64 attack = magic_lookup(piece, block, ROOK);
	debug_print_bitboard(block);
	debug_print_bitboard(attack);
	char fen[] = "1n6/2BrPPPp/1R1bp1k1/2QppNr1/1pn1b3/1p1PPppP/2P1BRP1/1qNK4 w kq - 0 1";
	Board board;
	BoardMove move;
	board_new(&board, fen);
	debug_print_board(&board);
	return 0;
}
