#include <stdio.h>
#include "include/board.h"
#undef NO_DEBUG
#define debug_print_board print_board
#include "include/debug.h"

int main(int argc, char **argv){
	char fen[] = "1n6/2BrPPPp/1R1bp1k1/2QppNr1/1pn1b3/1p1PPppP/2P1BRP1/1qNK4 w kq - 0 1";
	Board board;
	BoardMove move;
	board_new(&board, fen);
	print_board(&board);
	board_new(&board, NULL);
	print_board(&board);
	move.side = WHITE;
	move.piece = PAWN;
	move.promotion = PAWN;
	move.from = FILE_E & RANK_2;
	move.to   = FILE_E & RANK_4;
	board_play(&board, &move);
	print_board(&board);
	move.side = BLACK;
	move.piece = ROOK;
	move.promotion = ROOK;
	move.from = FILE_A & RANK_8;
	move.to   = FILE_C & RANK_5;
	board_play(&board, &move);
	print_board(&board);
	move.side = WHITE;
	move.piece = KING;
	move.promotion = QUEEN;
	move.from = FILE_E & RANK_1;
	move.to   = FILE_E & RANK_2;
	board_play(&board, &move);
	print_board(&board);
	move.side = BLACK;
	move.piece = BISHOP;
	move.promotion = BISHOP;
	move.from = FILE_F & RANK_8;
	board_play(&board, &move);
	print_board(&board);
	return 0;
}
