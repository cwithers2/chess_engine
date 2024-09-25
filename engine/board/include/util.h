#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

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

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef struct Board Board;
typedef struct BoardMove BoardMove;
typedef struct BoardMoveNode BoardMoveNode;

#define EMPTYSET 0ULL

u64 board_rand64();
u64 board_lprand64();
int board_ctz64(u64 value);

void board_format_pos(const u64 pos, char* str);
void board_format_move(const BoardMove* move, char* str);

#endif