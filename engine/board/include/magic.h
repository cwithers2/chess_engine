#ifndef MAGIC_H
#define MAGIC_H
#include "board.h"

int  magic_init();
void magic_destroy();

/*
	Parameters
	----------
	u64 piece:
		A single bit set on a bitboard. ie the piece we are looking up.
	u64 bboard:
		A bitboard of other pieces.
	int type:
		The type of the piece we are looking up (ROOK or BISHOP).

	Returns
	-------
	u64 attacks:
		A bitboard of pseudo legal attacks for piece based on block.
*/
u64  magic_lookup(u64 piece, u64 bboard, int type);
#endif
