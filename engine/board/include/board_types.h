#ifndef BOARD_TYPES_H
#define BOARD_TYPES_H

#define FEN_SKIP_1 '1'
#define FEN_SKIP_2 '2'
#define FEN_SKIP_3 '3'
#define FEN_SKIP_4 '4'
#define FEN_SKIP_5 '5'
#define FEN_SKIP_6 '6'
#define FEN_SKIP_7 '7'
#define FEN_SKIP_8 '8'
#define FEN_DASH   '-'
#define FEN_SLASH  '/'

enum FEN_FILES{
	#define X(F, BITS, CHAR) FEN_FILE_##F = CHAR,
	#include <x/file.h>
	#undef X
};

enum FEN_RANKS{
	#define X(R, BITS, CHAR) FEN_RANK_##R = CHAR,
	#include <x/rank.h>
	#undef X
};

enum FILES{
	#define X(F, BITS, CHAR) FILE_##F = BITS,
	#include <x/file.h>
	#undef X
};

enum RANKS{
	#define X(R, BITS, CHAR) RANK_##R = BITS,
	#include <x/rank.h>
	#undef X
};

enum SIDE_INDEX{
	#define X(SIDE, CHAR) SIDE,
	#include <x/side.h>
	#undef X
	SIDES
};
enum PIECE_INDEX{
	#define X(PIECE) PIECE,
	#include <x/piece.h>
	#undef X
	PIECES
};

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef struct Board Board;
typedef struct BoardMoveNode BoardMoveNode;

struct Board{
	u64  pieces[SIDES][PIECES];
	char castle[SIDES][2];
	char active;
	char target[2];
	u16  halfmoves;
	u16  fullmoves;
};

struct BoardMoveNode{
	u8 side;
	u64 beg;
	u64 end;
	u64 piece_beg;
	u64_piece_end;
	BoardMoveNode* next;
};

#endif
