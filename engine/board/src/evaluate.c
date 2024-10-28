#include <evaluate.h>
#include <lookup.h>

#define FOR_BIT_IN_SET(BIT, SET) \
for(u64 X=SET, BIT=X&-X; X; X&=(X-1), BIT=X&-X)

static const int checkmate_score[SIDES] = {INT_MAX, INT_MIN};

static const int piece_value[] = {
	#define X(PIECE, VALUE) VALUE,
	#include <x/piece.h>
	#undef X
};

static const u64 ranks[] = {
	#define X(RANK, BITS, CHAR) BITS,
	#include <x/rank.h>
	#undef X
};

static int clrsb(int);
static int evaluate(u64[SIDES][PIECES]);

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
	result  = evaluate(board->pieces);
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

static int evaluate(u64 pieces[SIDES][PIECES]){
	int material, mobility, king_safety, space, coordination;
	u64 bboard, sq, moves, allies, enemies, control, support;
	material     = 0;
	mobility     = 0;
	allies  = EMPTYSET;
	enemies = EMPTYSET;
	support = EMPTYSET;
	//get bboard
	for(int i = 0; i < PIECES; ++i){
		allies  |= pieces[WHITE][i];
		enemies |= pieces[BLACK][i];
	}
	bboard = allies | enemies;
	//calc material
	for(int i = 0; i < PIECES; ++i)
		material += (board_pop64(pieces[WHITE][i]) -
		             board_pop64(pieces[BLACK][i])) * piece_value[i];
	//calc pawn advancement advantage
	for(int i = 0; i < 8; ++i){
		u64 rank;
		int psum;
		rank  = pieces[WHITE][PAWN] & ranks[i];
		psum  = board_pop64(rank);
		rank  = pieces[BLACK][PAWN] & ranks[7-i];
		psum -= board_pop64(rank);
		material += psum * i * piece_value[PAWN];
	}
	//calc king safety
	moves = board_lookup(KING, pieces[WHITE][KING], bboard, WHITE);
	king_safety  = board_pop64(moves & allies);
	moves = board_lookup(KING, pieces[BLACK][KING], bboard, BLACK);
	king_safety -= board_pop64(moves & enemies);
	//calc individual values
	for(int i = 0; i < PIECES; ++i)
		FOR_BIT_IN_SET(sq, pieces[WHITE][i]){
			moves = board_lookup(i, sq, bboard, WHITE);
			mobility += board_pop64(moves);
			control  |= moves;
			support  |= moves & allies;
		}
	space = clrsb(board_pop64(control));
	coordination = board_pop64(support);
	control = EMPTYSET;
	support = EMPTYSET;
	for(int i = 0; i < PIECES; ++i)
		FOR_BIT_IN_SET(sq, pieces[BLACK][i]){
			moves = board_lookup(i, sq, bboard, BLACK);
			mobility -= board_pop64(moves);
			control  |= moves;
			support  |= moves & enemies;
		}
	space -= clrsb(board_pop64(control));
	coordination -= board_pop64(support);
	return material + mobility + king_safety + space + coordination;
}
