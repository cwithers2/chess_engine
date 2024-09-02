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

int board_fen_pieces(Board* board, const char* pieces){
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
	return 1;
}

int board_fen_active(Board* board, const char active){
	switch(active){
	#define X(SIDE, CHAR)\
	case CHAR:\
		board->active = CHAR;\
		break;
	#include <x/side.h>
	#undef X
	}
	return 1;
}

int board_fen_castle(Board* board, const char* castle){
	u8 indices[SIDES] = {0};
	u8 index;
	const char* ptr;
	if(castle[0] == FEN_DASH)
		return 1;
	ptr = castle;
	while(*ptr){
		switch(*ptr){
		#define X(SIDE, CHAR)\
		case CHAR:\
			index = indices[SIDE]++;\
			board->castle[SIDE][index] = CHAR;\
			break;
		#include <x/castle/white.h>
		#include <x/castle/black.h>
		#undef X
		#define X(SIDE, OLD, NEW)\
		case OLD:\
			index = indices[SIDE]++;\
			board->castle[SIDE][index] = NEW;\
			break;
		#include <x/castle/remap.h>
		#undef X
		}
		++ptr;
	}
	return 1;
}

int board_fen_target(Board* board, const char* target){
	if(target[0] == FEN_DASH)
		return 1;
	memcpy(board->target, target, 2);
	return 1;
}

int board_new(Board* board, const char* fen){
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

	return 1;
}

int board_copy(Board* copy, const Board* original){
	memcpy(copy, original, sizeof(Board));
	return 1;
}

int board_rem(Board* board, u8 side, u8 piece, u64 position){
	board->pieces[side][piece] &= ~position;
	return 1;
}

int board_set(Board* board, u8 side, u8 piece, u64 position){
	board->pieces[side][piece] |= position;
	return 1;
}

int board_clr(Board* board, u8 side, u64 position){
	#define X(PIECE) \
	board_rem(board, side, PIECE, position);
	#include <x/piece.h>
	#undef X
	return 1;
}

int board_play(Board* board, BoardMove* move){
	char file_lookup[] = {'A', 'a'};
	u64  rank_lookup[] = {RANK_1, RANK_8};
	u64 pos;
	int i;
	if(move->piece == KING)
		memset(board->castle[move->side], 0, 2);
	if(move->piece == ROOK){
		for(i = 0; i < 2; ++i){
			pos = rank_lookup[i] &
				(FILE_A * (file_lookup[i] - board->castle[move->side][i] + 1));
			if(pos == move->from)
				board->castle[move->side][i] = 0;
		}
	}
	board_rem(board, move->side, move->piece, move->from);
	board_set(board, move->side, move->promotion, move->to);
	board_clr(board, !(move->side), move->to);
	return 1;
}
