#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <board.h>

/** DEFINITIONS :) **/
typedef int64_t bitboard_t;

/** DATA TYPES :) **/
struct Board{
	bitboard_t pawns;
	bitboard_t knights;
	bitboard_t bishops;
	bitboard_t rooks;
	bitboard_t kings;
	bitboard_t enpassant;
	int halfmove_clock;
	int fullmove_number;
};

/** PUBLIC LIBRARY FUNCTIONS :) **/
Board* board_init(){
	Board* board;
	board = (Board*)malloc(sizeof(Board));
	return board;
}

void board_destroy(Board* board){
	free(board);
}

int board_set(Board* board, char* fen){
	if(!fen)
		fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	/*
	-tokenize(...)
	-'assert' 5 or 6 tokens (Enpassant token is technically optional)
	-set board
	-store castling rules
	-store enpassant rules
	-store halfmove clock
	-store fullmove number
	*/
}

int board_copy(Board* src, Board* dest){
	memcpy(dest, src, sizeof(Board));
}

int board_list(Board* board, BoardColor color, int* count, BoardMove** moves){
}

int board_move(BoardMove* move){
}
