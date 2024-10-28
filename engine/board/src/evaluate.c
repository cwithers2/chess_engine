#include <evaluate.h>
#include <lookup.h>
#include <debug.h>

#define debug_print_eval(X) debug_print("eval for %i " #X ": %i", side, X)

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
static int evaluate(u64[SIDES][PIECES], int side);

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
	result  = evaluate(board->pieces, WHITE);
	result -= evaluate(board->pieces, BLACK);
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
static int evaluate(u64 pieces[SIDES][PIECES], int side){
	int material, mobility, king_safety, space, coordination;
	u64 bboard, sq, moves, allies, enemies, control, support;
	material     = 0;
	mobility     = 0;
	allies  = EMPTYSET;
	enemies = EMPTYSET;
	support = EMPTYSET;
	control = EMPTYSET;
	//get bboard
	for(int i = 0; i < PIECES; ++i){
		allies  |= pieces[ side][i];
		enemies |= pieces[!side][i];
	}
	bboard = allies | enemies;
	//calc material
	for(int i = 0; i < PIECES; ++i)
		material += board_pop64(pieces[side][i]) * piece_value[i];
	//calc pawn advancement advantage
	switch(side){
	case WHITE:
		for(int i = 0; i < 8; ++i){
			u64 rank = pieces[WHITE][PAWN] & ranks[i];
			material += board_pop64(rank) * i * piece_value[PAWN];
		}
		break;
	default:
		for(int i = 0; i < 8; ++i){
			u64 rank = pieces[BLACK][PAWN] & ranks[7-i];
			material += board_pop64(rank) * i * piece_value[PAWN];
		}
		break;
	}
	//calc king safety
	moves = board_lookup(KING, pieces[side][KING], bboard, side);
	king_safety  = board_pop64(moves & allies);
	//calc individual values
	for(int i = 0; i < PIECES; ++i)
		FOR_BIT_IN_SET(sq, pieces[side][i]){
			moves = board_lookup(i, sq, bboard, side);
			mobility += board_pop64(moves);
			control  |= moves;
			support  |= moves & allies;
		}
	space = board_pop64(control);
	coordination = board_pop64(support);
	debug_print_eval(material);
	debug_print_eval(mobility);
	debug_print_eval(king_safety);
	debug_print_eval(space);
	debug_print_eval(coordination);
	return material + mobility + king_safety + space + coordination;
}
