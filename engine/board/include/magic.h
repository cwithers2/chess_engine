#ifndef MAGIC_H
#define MAGIC_H
#include "util.h"

int  board_magic_init();
void board_magic_destroy();

/*
	Parameters
	----------
	u64 piece:
		A single bit set on a bitboard. ie the piece we are looking up.
	u64 bboard:
		A bitboard of other pieces.
	int type:
		The type of the piece to lookup (ROOK or BISHOP).

	Returns
	-------
	u64 attacks:
		A bitboard of pseudo legal attacks for piece based on bboard.
*/
u64  board_magic_lookup(u64 piece, u64 bboard, int type);
#endif
