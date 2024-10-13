/* All algorithms in this file assume well formed FEN strings that
 * were already validated by some other operation (possibly the UCI library). */ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <board.h>
#include <lookup.h>
#include <debug.h>

//SECTION CONSTANTS & MACROS
#define FOR_BIT_IN_SET(BIT, SET) \
for(u64 X=SET, BIT=X&-X; X; X&=(X-1), BIT=X&-X)

#define LOOKUP(T, P, B, S) board_lookup(T, P, B, S)

#define QUEENSIDE       0
#define KINGSIDE        1
#define CASTLESIDE(K,C) (K > C)

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

const u64 PAWN_HOME[] = {RANK_2, RANK_7};
const u64 PAWN_PROM[] = {RANK_8, RANK_1};
const u64 PAWN_JUMP[] = {RANK_4, RANK_5};

int board_mode;

//SECTION: forward declarations
//SUBSECTION: fen parsing
static void fen_pieces(Board*, const char*);
static void fen_active(Board*, const char);
static void fen_castle(Board*, const char*);
static void fen_target(Board*, const char*);
//SUBSECTION: utility
static u64 flatten(u64*);
static int get_checkers(u64, u64, u64*, int, u64*, int*);
static u64 is_castle_move(u64, u64, u64);
static void get_rook_sides(u64, u64, u64*);
static void get_castling_squares(int, int, u64*, u64*);
static u64 advance_one(u64, int);
//SUBSECTION: node functions
static int push(BoardMove**, u64, u64, int, int);
static int push_move(BoardMove**, u64, u64, int, int);
static int push_moves(BoardMove**, u64, u64, int, int);
//SUBSECTION: move generation
static int gen_king_moves(u64, u64, u64*, u64, int, BoardMove**);
static int gen_castle_move(u64, u64, u64*, u64, int, int, BoardMove**);
static int gen_castle_moves(u64, u64, u64*, u64, int, BoardMove**);
static int gen_block_moves(u64, u64, u64, int, u64, int, u64, int, BoardMove**);
static int gen_nopin_moves(u64, u64, u64*, u64, int, u64, int, u64, int, BoardMove**);
static int gen_piece_moves(u64, u64, u64*, u64, int, u64, int, u64, int, BoardMove**);

//SECTION: function definitions
//SUBSECTION: public
int  board_init(int mode){
	board_mode = mode;
	return board_lookup_init();
}

void board_destroy(){
	board_lookup_destroy();
}

void board_new(Board* board, char* fen){
	int advance;
	const char* ptr;
	const char* head;
	char pieces[72];
	char active;
	char castle[5];
	char target[3];
	char space[2];
	u16 halfmoves;
	u16 fullmoves;
	memset(board, 0, sizeof(Board));
	if(!fen)
		head = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	else
		head = fen;
	ptr = head;
	#define SCAN(TYPE, VAL)\
	do{\
		sscanf(ptr, " %n" TYPE "%n", &advance, VAL, &advance);\
		ptr += advance;\
	}while(0)
	//PIECES
	SCAN("%71s", pieces);
	fen_pieces(board, pieces);
	//ACTIVE
	SCAN("%c", &active);
	fen_active(board, active);
	//CASTLE
	SCAN("%4s", castle);
	fen_castle(board, castle);
	//TARGET
	SCAN("%2s", target);
	fen_target(board, target);
	//HALFMOVES
	SCAN("%hu", &halfmoves);
	board->halfmoves = halfmoves;
	//FULLMOVES
	SCAN("%hu", &fullmoves);
	board->fullmoves = fullmoves;
	#undef SCAN
	debug_print_board(board);
}

void board_copy(Board* copy, Board* original){
	memcpy(copy, original, sizeof(Board));
}

void board_play(Board* board, BoardMove* move){
	u64 king_sq, rook_sq, rooks[2], rook, target;
	int castleside;
	#define side    (board->active)
	#define castles (board->castle[side])
	target = EMPTYSET;
	switch(move->piece_type){
	case PAWN:
		//check for target squares
		if( (move->from & PAWN_HOME[side]) &&
		    (move->to   & PAWN_JUMP[side]) )
			target = advance_one(move->from, side);
		if(move->to & board->target)
			board->pieces[!side][PAWN] &= ~(advance_one(move->to, !side));
		break;
	case KNIGHT:
	case BISHOP:
		break;
	case ROOK:
		//revoke applicable castling
		castles &= ~(move->from);
		break;
	case QUEEN:
		break;
	case KING:
		//check for castling move
		if(!castles)
			break;
		//check if we are castling
		if(is_castle_move(move->from, move->to, castles)){
			debug_print("%s", "Castling.");
			castleside = CASTLESIDE(move->from, move->to);
			get_castling_squares(side, castleside, &king_sq, &rook_sq);
			get_rook_sides(move->from, castles, rooks);
			board->pieces[side][KING] &= ~(move->from);
			board->pieces[side][ROOK] &= ~(rooks[castleside]);
			board->pieces[side][KING] |=   king_sq;
			board->pieces[side][ROOK] |=   rook_sq;
			board->castle[side] = EMPTYSET;
			goto UPDATE;
		}
		board->castle[side] = EMPTYSET;
		break;
	default:
		break;
	}
	//adjust pieces
	board->pieces[side][move->piece_type] &= ~(move->from);
	board->pieces[side][move->promotion]  |=   move->to;
	for(int i = 0; i < PIECES; ++i)
		board->pieces[!side][i] &= ~(move->to);
	UPDATE:
	//update counters and switch sides
	board->target = target;
	if(board->active == BLACK)
		board->fullmoves += 1;
	board->halfmoves += 1;
	board->active = !(side);
	#undef side
	#undef castles
}

int board_moves(Board* board, BoardMove* head){
	u64 checker, bboard, pmoves, sq, allies, pawn_bboard;
	int checker_type, side, status;
	BoardMove* tail;
	#define side    board->active
	#define enemies board->pieces[!(side)]
	#define king    board->pieces[side][KING]
	#define castles board->castle[side]
	#define state   board->state
	debug_print("%s", "King position:");
	debug_print_bitboard(king);
	debug_print("ctz(king): %i", board_ctz64(king));
	tail = head;
	allies = flatten(board->pieces[side]);
	bboard = allies | flatten(enemies);
	pawn_bboard = bboard | board->target;
	state = get_checkers(bboard, king, enemies, side, &checker, &checker_type);
	if(state == BOARD_NO_CHECK){
		status = gen_castle_moves(bboard, king, enemies, castles, side, &tail);
		if(status == BOARD_ERROR)
			goto ERROR;
	}
	pmoves = LOOKUP(KING, king, bboard, side) & ~allies;
	debug_print("%s", "Pseudo king moves:");
	debug_print_bitboard(pmoves);
	status = gen_king_moves(bboard, king, enemies, pmoves, side, &tail);
	if(status == BOARD_ERROR)
		goto ERROR;
	debug_print("%s", "Searching for moves.");
	if(state != BOARD_DOUBLE_CHECK)
		for(int i = 0; i < KING; ++i){
			FOR_BIT_IN_SET(sq, board->pieces[side][i]){
				switch(i){
				case PAWN:
					pmoves = LOOKUP(i, sq, pawn_bboard, side) & ~allies;
					break;
				default:
					pmoves = LOOKUP(i, sq, bboard, side) & ~allies;
					break;
				}
			status = gen_piece_moves(
				bboard, king, enemies, pmoves, side,
				sq, i, checker, checker_type, &tail);
			if(status == BOARD_ERROR)
				goto ERROR;
			}
		}
	debug_print("%s", "Done searching.");
	tail->next = NULL;
	if(head == tail)
		state |= BOARD_STALEMATE;
	return BOARD_SUCCESS;
	ERROR:
	board_moves_free(head->next);
	head->next = NULL;
	return BOARD_ERROR;
	#undef king
	#undef side
	#undef enemies
	#undef castles
	#undef state
}

void board_moves_free(BoardMove* moves){
	BoardMove* temp;
	while(moves){
		temp  = moves;
		moves = moves->next;
		free(temp);
	}
}
//SUBSECTION: fen parsing
static void fen_pieces(Board* board, const char* pieces){
	unsigned char rank;
	unsigned char file;
	const char* ptr;
	u64 focus;
	ptr = pieces;
	focus = 0x8000000000000000;
	for(rank = FEN_RANK_8; rank >= FEN_RANK_1; --rank){
		for(file = FEN_FILE_A; file <= FEN_FILE_H; ++file){
			switch(*ptr){
			#define X(SIDE, PIECE, CHAR)\
			case CHAR:\
				board->pieces[SIDE][PIECE] |= focus;\
				focus >>= 1;\
				break;
			#include <x/white.h>
			#include <x/black.h>
			#undef X
			#define CASE_NUM(CHAR)\
			case CHAR:\
				focus >>= CHAR - '0';\
				file += CHAR - '1';\
				break;
			CASE_NUM(FEN_SKIP_1);
			CASE_NUM(FEN_SKIP_2);
			CASE_NUM(FEN_SKIP_3);
			CASE_NUM(FEN_SKIP_4);
			CASE_NUM(FEN_SKIP_5);
			CASE_NUM(FEN_SKIP_6);
			CASE_NUM(FEN_SKIP_7);
			CASE_NUM(FEN_SKIP_8);
			#undef CASE_NUM
			}
			++ptr;
		}
		if(!*ptr)
			break;
		++ptr;
	}
}

static void fen_active(Board* board, const char active){
	switch(active){
	#define X(SIDE, CHAR)\
	case CHAR:\
		board->active = SIDE;\
		break;
	#include <x/side.h>
	#undef X
	}
}

static void fen_castle(Board* board, const char* castle){
	const char* ptr;
	if(castle[0] == FEN_DASH)
		return;
	ptr = castle;
	while(*ptr){
		switch(*ptr){
		#define X(SIDE, CHAR, POS)\
		case CHAR:\
			board->castle[SIDE] |= POS;\
			break;
		#include <x/castle.h>
		#include <x/castle_extended.h>
		#undef X
		}
		++ptr;
	}
}

static void fen_target(Board* board, const char* target){
	u64 rank, file;
	if(target[0] == FEN_DASH){
		board->target = EMPTYSET;
		return;
	}
	board->target = board_get_pos(target);
}

//SUBSECTION: utility
static u64 flatten(u64* pieces){
	u64 result = EMPTYSET;
	for(int i = 0; i < PIECES; ++i)
		result |= pieces[i];
	return result;
}

static void get_castling_squares(
	int side, int castleside,
	u64* king_sq, u64* rook_sq
){
	u64 home;
	home = PAWN_PROM[!side];
	switch(castleside){
	case QUEENSIDE:
		*king_sq = home & FILE_C;
		*rook_sq = home & FILE_D;
		break;
	default:
		*king_sq = home & FILE_G;
		*rook_sq = home & FILE_F;
		break;
	}
}

static void get_rook_sides(u64 king, u64 castles, u64* rooks){
	//rooks: rooks[0] == QUEENSIDE, rooks[1] == KINGSIDE
	switch(board_pop64(castles)){
	case 0:
		rooks[KINGSIDE]  = EMPTYSET;
		rooks[QUEENSIDE] = EMPTYSET;
		break;
	case 1:
		rooks[ CASTLESIDE(king, castles)] = castles;
		rooks[!CASTLESIDE(king, castles)] = EMPTYSET;
		break;
	default:
		rooks[KINGSIDE]  = castles & -castles;
		rooks[QUEENSIDE] = castles & ~rooks[KINGSIDE];
		break;
	}
}

static int get_checkers(
	u64 bboard, u64 king, u64* enemies, int side,
	u64* checker, int* checker_type
){
	u64 threat, threats;
	threats = EMPTYSET;
	int state = BOARD_NO_CHECK;
	*checker_type = BOARD_NO_CHECK;
	for(int i = 0; i < PIECES; ++i){
		switch(i){
		case QUEEN:
			threat = LOOKUP(BISHOP, king, bboard, side) & enemies[i];
			if(threat){
				*checker = threat;
				*checker_type = BISHOP;
				break;
			}
			threat = LOOKUP(ROOK, king, bboard, side)   & enemies[i];
			if(threat){
				*checker = threat;
				*checker_type = ROOK;
			}
			break;
		default:
			threat = LOOKUP(i, king, bboard, side) & enemies[i];
			if(threat){
				*checker = threat;
				*checker_type = i;
			}
		}
		threats |= threat;
	}
	switch(board_pop64(threats)){
	case 0:
		break;
	case 1:
		state = BOARD_CHECK;
		break;
	default:
		state = BOARD_DOUBLE_CHECK;
		break;
	}
	return state;
}

static u64 is_castle_move(u64 from, u64 to, u64 castles){
	u64 moves;
	switch(board_mode){
	case BOARD_MODE_960:
		return to & castles;
	default:
		moves = LOOKUP(KING, from, 0ULL, 0);
		return ~moves & to;
	}
}

static u64 advance_one(u64 bboard, int side){
	switch(side){
	case BLACK:
		return bboard >> 8;
	default:
		return bboard << 8;
	}
}


//SUBSECTION: node functions
static int push(
	BoardMove** tail, u64 from, u64 to, int piece_type, int promotion
){
	BoardMove *next;
	next = malloc(sizeof(BoardMove));
	if(!next)
		return BOARD_ERROR;
	*next = (BoardMove){
		.piece_type = piece_type,
		.promotion  = promotion,
		.from = from,
		.to   = to
	};
	(*tail)->next = next;
	*tail = next;
	return BOARD_SUCCESS;
}

static int push_move(
	BoardMove** tail, u64 from, u64 to, int piece_type, int side
){
	int count;
	count = 0;
	if(piece_type == PAWN && (to & PAWN_PROM[side])){
		debug_print("%s", "Promotion detected.");
		count += push(tail, from, to, PAWN, QUEEN);
		count += push(tail, from, to, PAWN, ROOK);
		count += push(tail, from, to, PAWN, BISHOP);
		count += push(tail, from, to, PAWN, KNIGHT);
		return (count == 4) ? BOARD_SUCCESS : BOARD_ERROR;
	}
	return push(tail, from, to, piece_type, piece_type);
}

static int push_moves(
	BoardMove** tail, u64 from, u64 moves, int piece_type, int side
){
	u64 sq;
	int count;
	count = 0;
	FOR_BIT_IN_SET(sq, moves)
		count += push_move(tail, from, sq, piece_type, side);
	return (count == board_pop64(moves)) ? BOARD_SUCCESS : BOARD_ERROR;
}
//SUBSECTION: move generation
static int gen_king_moves(
	u64 bboard, u64 king, u64* enemies, u64 pmoves, int side,
	BoardMove** tail
){
	int skip;
	u64 sq, check;
	FOR_BIT_IN_SET(sq, pmoves){
		skip = 0;
		for(int i = 0; i < PIECES; ++i){
			if(LOOKUP(i, sq, bboard, side) & enemies[i]){
				skip = 1;
				break;
			}
		}
		if(skip)
			continue;
		if(!push(tail, king, sq, KING, KING))
			return BOARD_ERROR;
	}
	return BOARD_SUCCESS;
}

static int gen_castle_move(
	u64 bboard, u64 king, u64* enemies, u64 rook, int side, int castleside,
	BoardMove** tail
){
	u64 to, king_sq, rook_sq, sq, slide, attack;
	get_castling_squares(side, castleside, &king_sq, &rook_sq);
	switch(board_mode){
	case BOARD_MODE_960:
		to = rook;
		break;
	default:
	case BOARD_MODE_STD:
		to = king_sq;
		break;
	}
	//check rook clearance
	if(rook_sq & bboard ||
	 !(LOOKUP(ROOK, rook, bboard & ~king, side) & rook_sq))
		return BOARD_SUCCESS;
	//check king clearance
	if(king_sq & bboard ||
	 !(LOOKUP(ROOK, king, bboard & ~rook, side) & king_sq))
	 	return BOARD_SUCCESS;
	//check attackers
	slide = ( LOOKUP(ROOK, king,    bboard, side) &
	          LOOKUP(ROOK, king_sq, bboard, side) ) | king_sq;
	FOR_BIT_IN_SET(sq, slide){
		for(int i = 0; i < PIECES; ++i){
			attack = LOOKUP(i, sq, bboard, side) & enemies[i];
			if(attack)
				return BOARD_SUCCESS;
		}
	}
	return push_move(tail, king, to, KING, side);
}

static int gen_castle_moves(
	u64 bboard, u64 king, u64* enemies, u64 castles, int side,
	BoardMove** tail
){
	u64 rooks[2];
	int result;
	switch(board_pop64(castles)){
	case 0:
		return BOARD_SUCCESS;
		break;
	case 1:
		return gen_castle_move(
			bboard, king, enemies, castles, side, CASTLESIDE(king, castles),
			tail);
	default:
		get_rook_sides(king, castles, rooks);
		result  = 0;
		result += gen_castle_move(
			bboard, king, enemies, rooks[QUEENSIDE], side, QUEENSIDE, tail);
		result += gen_castle_move(
			bboard, king, enemies, rooks[KINGSIDE],  side, KINGSIDE,  tail);
		return result == 2 ? BOARD_SUCCESS : BOARD_ERROR;
	}
}

static int gen_block_moves(
	u64 bboard, u64 king, u64 pmoves, int side,
	u64 piece, int piece_type, u64 checker, int checker_type,
	BoardMove** tail
){
	u64 blocks, moves;
	blocks = LOOKUP(checker_type, checker, bboard, side) &
	         LOOKUP(checker_type, king,    bboard, side);
	moves  = pmoves & (blocks | checker);
	return push_moves(tail, piece, moves, piece_type, side);
}

static int gen_nopin_moves(
	u64 bboard, u64 king, u64* enemies, u64 pmoves, int side,
	u64 piece, int piece_type, u64 checker, int checker_type,
	BoardMove** tail
){
	u64 exclude, moves, blocks, attacks, pinner;
	exclude = bboard & ~piece;
	//find bishop like pins
	attacks = LOOKUP(BISHOP, king, exclude, side);
	pinner  = attacks & (enemies[BISHOP] | enemies[QUEEN]);
	//assert popcount(pinner) <= 1;
	if(pinner){
		blocks = LOOKUP(BISHOP, pinner, exclude, side) & attacks;
		moves  = pmoves & (blocks | pinner);
		return push_moves(tail, piece, moves, piece_type, side);
	}
	//find rook like pins
	attacks = LOOKUP(ROOK, king, exclude, side);
	pinner  = attacks & (enemies[ROOK]   | enemies[QUEEN]);
	//assert popcount(pinner) <= 1
	if(pinner){
		blocks = LOOKUP(ROOK, pinner, exclude, side)   & attacks;
		moves  = pmoves & (blocks | pinner);
		return push_moves(tail, piece, moves, piece_type, side);
	}
	//no pins found
	return push_moves(tail, piece, pmoves, piece_type, side);
}

static int gen_piece_moves(
	u64 bboard, u64 king, u64* enemies, u64 pmoves, int side,
	u64 piece, int piece_type, u64 checker, int checker_type,
	BoardMove** tail
){
	switch(checker_type){
	case BOARD_NO_CHECK:
		//find moves that agree with any/all pins.
		return gen_nopin_moves(
			bboard, king, enemies, pmoves, side,
			piece, piece_type, checker, checker_type,
			tail);
	case BISHOP:
	case ROOK:
		//we are checked, but we can block.
		return gen_block_moves(
			bboard, king, pmoves, side,
			piece, piece_type, checker, checker_type,
			tail);
	case PAWN:
	case KNIGHT:
		//we are checked, and we cannot block.
		if(pmoves & checker)
			return push_move(tail, piece, checker, piece_type, side);
		return BOARD_SUCCESS;
	case KING:
		//this is not normal.
	case QUEEN:
		//this should have already been converted to rook or bishop.
	default:
		return BOARD_SUCCESS;
	}
}
