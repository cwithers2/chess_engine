#include <evaluate.h>
#include <lookup.h>

#define FOR_BIT_IN_SET(BIT, SET) \
for(u64 X=SET, BIT=X&-X; X; X&=(X-1), BIT=X&-X)

const int checkmate_score[SIDES] = {INT_MAX, INT_MIN};

const int piece_value[] = {
	#define X(PIECE, VALUE) VALUE,
	#include <x/piece.h>
	#undef X
};

const u64 ranks[] = {
	#define X(RANK, BITS, CHAR) BITS,
	#include <x/rank.h>
	#undef X
};

static int clrsb(int);
static int evaluate(u64**, int);

int board_evaluate(Board* board){
	#define side (board->active)
	int result;
	u64 bboard;
	switch(board->state){
	case BOARD_STALEMATE | BOARD_CHECK:
	case BOARD_STALEMATE | BOARD_DOUBLE_CHECK:
		return checkmate_score[!side];
	case BOARD_STALEMATE:
		return 0;
	default:
		break;
	}
	result  = evaluate((u64**)(board->pieces), WHITE);
	result -= evaluate((u64**)(board->pieces), BLACK);
	return result;
	#undef side
}

//count leading redundant signed bits
static int clrsb(int x){
#ifdef __GNUC__
	return __builtin_clrsb(x);
#else
#error Function not defined for this compiler.
#endif
}

static int evaluate(u64** pieces, int side){
	int material, mobility, king_safety, space, coordination;
	u64 bboard, sq, moves, allies, enemies, control, support;
	material     = 0;
	mobility     = 0;
	allies  = EMPTYSET;
	enemies = EMPTYSET;
	support = EMPTYSET;
	//get bboard
	for(int i = 0; i < PIECES; ++i){
		allies  |= pieces[ side][i];
		enemies |= pieces[!side][i];
	}
	bboard = allies | enemies;
	//calc material
	for(int i = 0; i < PIECES; ++i)
		material += board_pop64(pieces[side][i]) * piece_value[i];
	switch(side){
	case WHITE:
		for(int i = 0; i < 8; ++i){
			u64 rank = (pieces[WHITE][PAWN] >> (8*i)) & ranks[0];
			material += i * board_ctz64(rank);
		}
		break;
	default:
		for(int i = 7; i >= 0; --i){
			u64 rank = (pieces[BLACK][PAWN] >> (8*i)) & ranks[0];
			material += i * board_ctz64(rank);
		}
	}
	//calc king safety
	moves = board_lookup(KING, pieces[side][KING], bboard, side);
	king_safety = board_pop64(moves & allies);
	//calc individual values
	for(int i = 0; i < PIECES; ++i)
		FOR_BIT_IN_SET(sq, pieces[side][i]){
			moves = board_lookup(i, sq, bboard, side);
			mobility += board_pop64(moves);
			control  |= moves;
			support  |= moves & allies;
		}
	space = clrsb(board_pop64(control));
	coordination = board_pop64(support);
	return material + mobility + king_safety + space + coordination;
}
