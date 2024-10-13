#ifndef MAGIC_H
#define MAGIC_H
#include "util.h"

/*
	Returns
	-------
	int result:
		BOARD_SUCCESS or BOARD_ERROR.
*/
int  board_lookup_init();
void board_lookup_destroy();

/*
	Lookup the squares a piece can move to based on a bitboard.

	NOTE: All pieces described in bboard are considered enemies.
	      An additional bitwise op is needed to eliminate allies.

	Parameters
	----------
	int type:
		The type of piece to find moves for:
			PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
	u64 piece:
		A single bit denoting where the piece is on the board.
	u64 bboard:
		Any amount of bits denoting 'enemy' pieces.
	int side:
		The side of the piece: WHITE or BLACK.
		NOTE: only used by pawn lookups.
	Returns
	-------
	u64 bit board of moves and captures.
*/
u64  board_lookup(int type, u64 piece, u64 bboard, int side);
#endif
