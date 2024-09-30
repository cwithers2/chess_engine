/* All algorithms in this file assume well formed FEN strings that
 * were already validated by some other operation (possibly the UCI library). */ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <board.h>
#include <magic.h>
#include <debug.h>

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

u64 ktable[64] = {
	0x0000000000000302ULL, 0x0000000000000705ULL,
	0x0000000000000E0AULL, 0x0000000000001C14ULL,
	0x0000000000003828ULL, 0x0000000000007050ULL,
	0x000000000000E0A0ULL, 0x000000000000C040ULL,
	0x0000000000030203ULL, 0x0000000000070507ULL,
	0x00000000000E0A0EULL, 0x00000000001C141CULL,
	0x0000000000382838ULL, 0x0000000000705070ULL,
	0x0000000000E0A0E0ULL, 0x0000000000C040C0ULL,
	0x0000000003020300ULL, 0x0000000007050700ULL,
	0x000000000E0A0E00ULL, 0x000000001C141C00ULL,
	0x0000000038283800ULL, 0x0000000070507000ULL,
	0x00000000E0A0E000ULL, 0x00000000C040C000ULL,
	0x0000000302030000ULL, 0x0000000705070000ULL,
	0x0000000E0A0E0000ULL, 0x0000001C141C0000ULL,
	0x0000003828380000ULL, 0x0000007050700000ULL,
	0x000000E0A0E00000ULL, 0x000000C040C00000ULL,
	0x0000030203000000ULL, 0x0000070507000000ULL,
	0x00000E0A0E000000ULL, 0x00001C141C000000ULL,
	0x0000382838000000ULL, 0x0000705070000000ULL,
	0x0000E0A0E0000000ULL, 0x0000C040C0000000ULL,
	0x0003020300000000ULL, 0x0007050700000000ULL,
	0x000E0A0E00000000ULL, 0x001C141C00000000ULL,
	0x0038283800000000ULL, 0x0070507000000000ULL,
	0x00E0A0E000000000ULL, 0x00C040C000000000ULL,
	0x0302030000000000ULL, 0x0705070000000000ULL,
	0x0E0A0E0000000000ULL, 0x1C141C0000000000ULL,
	0x3828380000000000ULL, 0x7050700000000000ULL,
	0xE0A0E00000000000ULL, 0xC040C00000000000ULL,
	0x0203000000000000ULL, 0x0507000000000000ULL,
	0x0A0E000000000000ULL, 0x141C000000000000ULL,
	0x2838000000000000ULL, 0x5070000000000000ULL,
	0xA0E0000000000000ULL, 0x40C0000000000000ULL
};

u64 ntable[64] = {
	0x0000000000020400ULL, 0x0000000000050800ULL,
	0x00000000000A1100ULL, 0x0000000000142200ULL,
	0x0000000000284400ULL, 0x0000000000508800ULL,
	0x0000000000A01000ULL, 0x0000000000402000ULL,
	0x0000000002040004ULL, 0x0000000005080008ULL,
	0x000000000A110011ULL, 0x0000000014220022ULL,
	0x0000000028440044ULL, 0x0000000050880088ULL,
	0x00000000A0100010ULL, 0x0000000040200020ULL,
	0x0000000204000402ULL, 0x0000000508000805ULL,
	0x0000000A1100110AULL, 0x0000001422002214ULL,
	0x0000002844004428ULL, 0x0000005088008850ULL,
	0x000000A0100010A0ULL, 0x0000004020002040ULL,
	0x0000020400040200ULL, 0x0000050800080500ULL,
	0x00000A1100110A00ULL, 0x0000142200221400ULL,
	0x0000284400442800ULL, 0x0000508800885000ULL,
	0x0000A0100010A000ULL, 0x0000402000204000ULL,
	0x0002040004020000ULL, 0x0005080008050000ULL,
	0x000A1100110A0000ULL, 0x0014220022140000ULL,
	0x0028440044280000ULL, 0x0050880088500000ULL,
	0x00A0100010A00000ULL, 0x0040200020400000ULL,
	0x0204000402000000ULL, 0x0508000805000000ULL,
	0x0A1100110A000000ULL, 0x1422002214000000ULL,
	0x2844004428000000ULL, 0x5088008850000000ULL,
	0xA0100010A0000000ULL, 0x4020002040000000ULL,
	0x0400040200000000ULL, 0x0800080500000000ULL,
	0x1100110A00000000ULL, 0x2200221400000000ULL,
	0x4400442800000000ULL, 0x8800885000000000ULL,
	0x100010A000000000ULL, 0x2000204000000000ULL,
	0x0004020000000000ULL, 0x0008050000000000ULL,
	0x00110A0000000000ULL, 0x0022140000000000ULL,
	0x0044280000000000ULL, 0x0088500000000000ULL,
	0x0010A00000000000ULL, 0x0020400000000000ULL
};

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
	switch(target[0]){
	#define X(FILE, BITS, CHAR)\
	case CHAR:\
		file = BITS;\
		break;
	#include <x/file.h>
	#undef X
	}
	switch(target[1]){
	#define X(RANK, BITS, CHAR)\
	case CHAR:\
		rank = BITS;\
		break;
	#include <x/rank.h>
	#undef X
	}
	board->target = file & rank;
}

static u64 pawn_advance(u64 piece, int side){
	switch(side){
	case WHITE:
		return piece << 8;
	case BLACK:
		return piece >> 8;
	};
}

static u64 pawn_lookup(u64 piece, u64 bboard, int side){
	u64 result, attack;
	result = 0ULL;
	//ignore promotion squares
	if(piece & PAWN_PROM[side])
		return result;
	//single and double advance
	if(!(pawn_advance(piece, side) & bboard)){
		result |= pawn_advance(piece, side);
		if((piece & PAWN_HOME[side]) && !(pawn_advance(result, side) & bboard))
			result |= pawn_advance(result, side);
	}
	//diagonal left
	if(!(piece & FILE_A)){
		attack = pawn_advance(piece << 1, side);
		if(attack & bboard)
			result |= attack;
	}
	//diagonal right
	if(!(piece & FILE_H)){
		attack = pawn_advance(piece >> 1, side);
		if(attack & bboard)
			result |= attack;
	}
	return result;
}

static u64 flatten(u64* pieces){
	u64 result = EMPTYSET;
	for(int i = 0; i < PIECES; ++i)
		result |= pieces[i];
	return result;
}

static u64 lookup(u64 piece, u64 bboard, int type, int side){
	u64 result;
	switch(type){
	case BISHOP:
	case ROOK:
	case QUEEN:
		result = board_magic_lookup(piece, bboard, type);
		break;
	case KING:
		result = ktable[board_ctz64(piece)];
		break;
	case KNIGHT:
		result = ntable[board_ctz64(piece)];
		break;
	case PAWN:
		result = pawn_lookup(piece, bboard, side);
		break;
	default:
		break;
	}
	return result;
}

//global functions
int  board_init(){
	return board_magic_init();
}

void board_destroy(){
	board_magic_destroy();
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
	//castle tracking
	if(move->piece_type == KING)
		board->castle[board->active] = EMPTYSET;
	if(move->piece_type == ROOK)
		board->castle[board->active] &= ~(move->from);
	//en passant tracking
	board->target = EMPTYSET;
	if(move->piece_type == PAWN)
		if( (move->from & PAWN_HOME[board->active]) &&
		    (move->to   & PAWN_JUMP[board->active]) )
			board->target = move->to;
	//capturing opponent's castle
	if(move->to & board->castle[!(board->active)])
		board->castle[!(board->active)] &= ~(move->to);
	//adjust our pieces
	board->pieces[board->active][move->piece_type] &= ~(move->from);
	board->pieces[board->active][move->promotion]  |=   move->to;
	//record any captures of enemy pieces
	for(int i = 0; i < PIECES; ++i)
		board->pieces[!(board->active)][i] &= ~(move->to);
	//update counters and switch sides
	if(board->active == BLACK)
		board->fullmoves += 1;
	board->halfmoves += 1;
	board->active = !(board->active);
}

#define FOR_BIT_IN_SET(BIT, SET) \
for(u64 X=SET, BIT=X&-X; X; X&=(X-1), BIT=X&-X)
#define DOUBLE_CHECK 11
#define NO_CHECK     12

static int push(
	BoardMove** tail, u64 from, u64 to, int piece_type, int promotion
){
	(*tail)->next = malloc(sizeof(BoardMove));
	if(!((*tail)->next))
		return BOARD_ERROR;
	*((*tail)->next) = (BoardMove){
		.piece_type = piece_type,
		.promotion  = promotion,
		.from = from,
		.to   = to
	};
	tail = &((*tail)->next);
	return BOARD_SUCCESS;
}

static int push_move(
	BoardMove** tail, u64 from, u64 to, int piece_type, int side
){
	int count;
	count = 0;
	if(piece_type == PAWN && (to & PAWN_PROM[side])){
		count += push(tail, from, to, PAWN, QUEEN);
		count += push(tail, from, to, PAWN, BISHOP);
		count += push(tail, from, to, PAWN, KNIGHT);
		count += push(tail, from, to, PAWN, ROOK);
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
		count += push(tail, from, sq, piece_type, side);
	return (count == board_pop64(moves)) ? BOARD_SUCCESS : BOARD_ERROR;
}

static int gen_king_moves(
	u64 bboard, u64 king, u64* enemies, u64 pmoves, int side,
	BoardMove** tail
){
	//todo impliment castling
	int status;
	u64 sq, check;
	FOR_BIT_IN_SET(sq, pmoves){
		for(int i = 0; i < PIECES; ++i){
			check = lookup(sq, bboard, i, side) & enemies[i];
			if(!check){
				status = push(tail, king, sq, KING, KING);
				if(!status)
					return BOARD_ERROR;
			}
		}
	}
	return BOARD_SUCCESS;
}

static void get_checkers(
	u64 bboard, u64 king, u64* enemies, int side,
	u64* checker, int* checker_type
){
	u64 threat;
	*checker_type = NO_CHECK;
	for(int i = 0; i < PIECES; ++i){
		threat = lookup(king, bboard, i, side) & enemies[i];
		if(threat)
			*checker_type = i;
		*checker |= threat;
	}
	if(board_pop64(*checker) > 1)
		*checker_type = DOUBLE_CHECK;
}

static int gen_block_moves(
	u64 bboard, u64 king, u64 pmoves, int side,
	u64 piece, int piece_type, u64 checker, int checker_type,
	BoardMove** tail
){
	u64 blocks, moves;
	blocks = board_magic_lookup(checker, bboard, checker_type) &
	         board_magic_lookup(king,    bboard, checker_type);
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
	attacks = board_magic_lookup(king, exclude, BISHOP);
	pinner  = attacks & (enemies[BISHOP] | enemies[QUEEN]);
	//assert popcount(pinner) <= 1;
	if(pinner){
		blocks = board_magic_lookup(pinner, exclude, BISHOP) & attacks;
		moves  = pmoves & (blocks | pinner);
		return push_moves(tail, piece, moves, piece_type, side);
	}
	//find rook like pins
	attacks = board_magic_lookup(king, exclude, ROOK);
	pinner  = attacks & (enemies[ROOK]   | enemies[QUEEN]);
	//assert popcount(pinner) <= 1
	if(pinner){
		blocks = board_magic_lookup(pinner, exclude, ROOK)   & attacks;
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
	case BISHOP:
	case ROOK:
	case QUEEN:
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
	case NO_CHECK:
		//find moves that agree with any/all pins.
		return gen_nopin_moves(
			bboard, king, enemies, pmoves, side,
			piece, piece_type, checker, checker_type,
			tail);
	case DOUBLE_CHECK:
		//only king moves can avoid this.
	case KING:
		//this is not normal.
	default:
		return BOARD_SUCCESS;
	}
}

int board_moves(Board* board, BoardMove** tail){
	u64 checker, bboard, pmoves, sq, allies;
	int checker_type, side, status;
	BoardMove* head;
	head = *tail;
	#define side    board->active
	#define enemies board->pieces[!(side)]
	#define king    board->pieces[side][KING]
	allies = flatten(board->pieces[side]);
	bboard = allies | flatten(enemies);
	get_checkers(bboard, king, enemies, side, &checker, &checker_type);
	pmoves = lookup(bboard, king, KING, side) & ~allies;
	status = gen_king_moves(bboard, king, enemies, pmoves, side, tail);
	if(!status)
		goto ERROR;
	if(checker_type != DOUBLE_CHECK)
		for(int i = 0; i < PIECES; ++i)
			FOR_BIT_IN_SET(sq, board->pieces[side][i]){
				pmoves = lookup(bboard, sq, i, side) & ~allies;
				status = gen_piece_moves(
					bboard, king, enemies, pmoves, side,
					sq, i, checker, checker_type, tail);
				if(!status)
					goto ERROR;
			}
	if(head == *tail)
		if(checker_type == NO_CHECK){
			return BOARD_STALEMATE;
		}else{
			return BOARD_CHECKMATE;
		}
	return BOARD_SUCCESS;
	ERROR:
	board_moves_free(head->next);
	return BOARD_ERROR;
}

void board_moves_free(BoardMove* head){
	BoardMove* temp;
	while(head){
		temp = head;
		head = head->next;
		free(temp);
	}
}
