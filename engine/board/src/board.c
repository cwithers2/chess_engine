/* All algorithms in this file assume well formed FEN strings that
 * were already validated by some other operation (possibly the UCI library). */ 
#include <board.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void board_fen_pieces(Board* board, const char* pieces){
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

void board_fen_active(Board* board, const char active){
	switch(active){
	#define X(SIDE, CHAR)\
	case CHAR:\
		board->active = SIDE;\
		break;
	#include <x/side.h>
	#undef X
	}
}

void board_fen_castle(Board* board, const char* castle){
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

void board_fen_target(Board* board, const char* target){
	u64 rank, file;
	if(target[0] == FEN_DASH){
		board->target = 0;
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

void board_new(Board* board, const char* fen){
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
	board_fen_pieces(board, pieces);
	//ACTIVE
	SCAN("%c", &active);
	board_fen_active(board, active);
	//CASTLE
	SCAN("%4s", castle);
	board_fen_castle(board, castle);
	//TARGET
	SCAN("%2s", target);
	board_fen_target(board, target);
	//HALFMOVES
	SCAN("%hu", &halfmoves);
	board->halfmoves = halfmoves;
	//FULLMOVES
	SCAN("%hu", &fullmoves);
	board->fullmoves = fullmoves;
	#undef SCAN
	debug_print_board(board);
}

void board_copy(Board* copy, const Board* original){
	memcpy(copy, original, sizeof(Board));
}

void board_play(Board* board, const BoardMove* move){
	u64 home_lookup[] = {RANK_2, RANK_7};
	u64 move_lookup[] = {RANK_4, RANK_5};
	int i;
	if(move->piece == KING)
		board->castle[board->active] = 0;
	if(move->piece == ROOK)
		for(i = 0; i < 2; ++i)
			if(board->castle[board->active] & move->from)
				board->castle[board->active] &= ~(move->from);
	board->target = 0;
	if(move->piece == PAWN)
		if( (move->from & home_lookup[board->active]) &&
		    (move->to   & move_lookup[board->active]) )
			board->target = move->to;
	if(move->to & board->castle[!(board->active)])
		board->castle[!(board->active)] &= ~(move->to);
	board->pieces[board->active][move->piece] &= ~(move->from);
	board->pieces[board->active][move->promotion] |= move->to;
	#define X(PIECE) \
	board->pieces[!(board->active)][PIECE] &= ~(move->to);
	#include <x/piece.h>
	#undef X
	if(board->active == BLACK)
		board->fullmoves += 1;
	board->halfmoves += 1;
	board->active = !(board->active);
}

void board_format_pos(const u64 pos, char* str){
	#define X(FILE, BITS, CHAR)\
	if(pos & BITS) str[0] = CHAR;
	#include <x/file.h>
	#undef X
	#define X(RANK, BITS, CHAR)\
	if(pos & BITS) str[1] = CHAR;
	#include <x/rank.h>
	#undef X
}

void board_format_move(const BoardMove* move, char* str){
	board_format_pos(move->from, str);
	board_format_pos(move->to, str+2);
}

u64 board_flatten(const Board* board, const u8 side){
	u64 bboard = 0;
	#define X(PIECE)\
	bboard |= board->pieces[side][PIECE];
	#include <x/piece.h>
	#undef X
	return bboard;
}

