#ifndef BOARD_H
#define BOARD_H

enum FILES{
	#define X(F, BITS, CHAR) FILE_##F = BITS,
	#include "x/file.h"
	#undef X
};

enum RANKS{
	#define X(R, BITS, CHAR) RANK_##R = BITS,
	#include "x/rank.h"
	#undef X
};

enum SIDE_INDEX{
	#define X(SIDE, CHAR) SIDE,
	#include "x/side.h"
	#undef X
	SIDES
};
enum PIECE_INDEX{
	#define X(PIECE) PIECE,
	#include "x/piece.h"
	#undef X
	PIECES
};

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef struct Board Board;
typedef struct BoardMove BoardMove;

struct Board{
	u64  pieces[SIDES][PIECES];
	char castle[SIDES][2];
	char active;
	char target[2];
	u16  halfmoves;
	u16  fullmoves;
};

struct BoardMove{
	u8 side;
	u8 piece;
	u8 promotion;
	u64 from;
	u64 to;
};

int board_new(Board* board, const char* fen);
int board_copy(Board* copy, const Board* original);
int board_play(Board* board, BoardMove* move);

#endif
