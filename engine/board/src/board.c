#include <board.h>
#include <string.h>
#include <stdlib.h>

#include <debug.h>

//#define POS(F,R) F&R
/*
struct BoardMove{
	u8  side;
	u8  beg_piece;
	u8  end_piece;
	u64 beg;
	u64 end;
};

typedef struct BoardPieceNode BoardPieceNode;
struct BoardPiece{
	u8  piece;
	u64 place;
	u64 moves;
};
struct BoardPieceNode{
	BoardPieceNode* next;
	BoardPiece data;
};*/

u64 board_bitboard(Board* board, u8 side){
	u64 result = 0;
	switch(side){
	#define X(SIDE, PIECE, CHAR) result |= board->pieces[SIDE][PIECE];
	case WHITE:
		#include <x/white.h>
		break;
	case BLACK:
		#include <x/black.h>
		break;
	default:
		#include <x/white.h>
		#include <x/black.h>
		break;
	#undef X
	}
	return result;
}

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
			default:
				debug_print(
					"Unidentified piece '%c' at %c%c.",
					*ptr, file, rank);
				goto ABORT;
			}
			++ptr;
		}
		if(file-1 > FEN_FILE_H){
			debug_print("File out of bounds by %i.", file-1-FEN_FILE_H);
			goto ABORT;
		}
		if(!*ptr)
			break;
		if(*ptr != FEN_SLASH){
			debug_print("Expected '%c', but got: %c.", FEN_SLASH, *ptr);
			goto ABORT;
		}
		++ptr;
	}
	if(rank < FEN_RANK_1){
		debug_print("%s", "Too many ranks defined.");
		goto ABORT;
	}
	if(rank > FEN_RANK_1){
		debug_print("Missing data for rank %c.", rank-1);
		goto ABORT;
	}
	return 1;
	ABORT:
	return 0;
}

int board_fen_active(Board* board, const char active){
	switch(active){
	#define X(SIDE, CHAR)\
	case CHAR:\
		board->active = CHAR;\
		break;
	#include <x/side.h>
	#undef X
	default:
		debug_print("Unidentified active player '%c'.", active);
		goto ABORT;
	}
	return 1;
	ABORT:
	return 0;
}

int board_fen_castle(Board* board, const char* castle){
	u8 indices[SIDES] = {0};
	char cache[SIDES] = {0};
	u8 index;
	size_t len;
	const char* ptr;
	len = strlen(castle);
	if(len == 1 && *castle == FEN_DASH)
		return 1;
	if(len > 4){
		//error
		goto ABORT;
	}
	ptr = castle;
	while(*ptr){
		switch(*ptr){
		#define X(SIDE, CHAR)\
		case CHAR:\
			if(CHAR == cache[SIDE]) goto OVERFLOW;\
			cache[SIDE] = CHAR;\
			index = indices[SIDE]++;\
			if(index >= 2) goto OVERFLOW;\
			board->castle[SIDE][index] = CHAR;\
			break;
		#include <x/castle/white.h>
		#include <x/castle/black.h>
		#undef X
		#define X(SIDE, OLD, NEW)\
		case OLD:\
			if(NEW == cache[SIDE]) goto OVERFLOW;\
			cache[SIDE] = NEW;\
			index = indices[SIDE]++;\
			if(index >= 2) goto OVERFLOW;\
			board->castle[SIDE][index] = NEW;\
			break;
		#include <x/castle/remap.h>
		#undef X
		default:
			debug_print("Unidentified castle '%c'.", *ptr);
			goto ABORT;
		}
		++ptr;
	}
	return 1;
	OVERFLOW:
	debug_print("Overflow castle '%c'.", *ptr);
	ABORT:
	return 0;
}

int board_fen_target(Board* board, const char* target){
	if(!target[1] && *target == FEN_DASH)
		return 1;
	if((target[0] <  FEN_FILE_A)||(target[0] >  FEN_FILE_H))
		goto ABORT;
	if((target[1] != FEN_RANK_3)&&(target[1] != FEN_RANK_6))
		goto ABORT;
	memcpy(board->target, target, 2);
	return 1;
	ABORT:
	debug_print("Invalid en passant target %s.", target);
	return 0;
}

#define SCAN_VALUE_OR_ERROR(TYPE, VAL, MESG)\
do{\
	if(!sscanf(ptr, " %n" TYPE "%n", &advance, VAL, &advance)){\
		ptr += advance;\
		debug_print("%s", MESG);\
		goto ERROR;\
	}\
	ptr += advance;\
}while(0)
#define SCAN_SPACE_OR_ERROR(MESG) \
do{\
	if(*ptr && sscanf(ptr, "%1[^ \t]", space)){\
		debug_print("%s", MESG);\
		goto ERROR;\
	}\
}while(0)
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
	debug_print("%s", "Creating new board object.");
	memset(board, NULL, sizeof(Board));
	if(!fen)
		head = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	else
		head = fen;
	ptr = head;
	//PIECES
	SCAN_VALUE_OR_ERROR("%71s", pieces, "Cannot not read piece field:");
	SCAN_SPACE_OR_ERROR("Extra piece data:");
	if(!board_fen_pieces(board, pieces))
		goto PARSE;
	//ACTIVE
	SCAN_VALUE_OR_ERROR("%c", &active, "Cannot not read active field:");
	SCAN_SPACE_OR_ERROR("Extra active data:");
	if(!board_fen_active(board, active))
		goto PARSE;
	//CASTLE
	SCAN_VALUE_OR_ERROR("%4s", castle, "Cannot not read castle field:");
	SCAN_SPACE_OR_ERROR("Extra castle data:");
	if(!board_fen_castle(board, castle))
		goto PARSE;
	//TARGET
	SCAN_VALUE_OR_ERROR("%2s", target, "Cannot not read target field:");
	SCAN_SPACE_OR_ERROR("Extra target data:");
	if(!board_fen_target(board, target))
		goto PARSE;
	//HALFMOVES
	SCAN_VALUE_OR_ERROR("%hu", &halfmoves, "Cannot not read halfmoves field:");
	SCAN_SPACE_OR_ERROR("Extra halfmoves data:");
	board->halfmoves = halfmoves;
	//FULLMOVES
	SCAN_VALUE_OR_ERROR("%hu", &fullmoves, "Cannot not read fullmoves field:");
	SCAN_SPACE_OR_ERROR("Extra fullmoves data:");
	board->fullmoves = fullmoves;

	debug_print_board(board);

	return 1;
	PARSE:
	--ptr;
	debug_print("%s", "In this field:");
	ERROR:
	debug_print("%*cv", (int)(ptr-head), ' ');
	debug_print("%s", head);
	ABORT:
	return 0;
}
#undef SCAN_VALUE_OR_ERROR
#undef SCAN_SPACE_OR_ERROR
//test
int board_copy(Board* copy, const Board* original){
	return memcpy(copy, original, sizeof(Board));
}

u64 fill(u64 attackers){
}

u64 occludedFill(u64 attackers, u64 blockers){
}

//Kogge-Stone algorithm
#define KOGGE_STONE(OP, MASK, V1, V2, V3)\
empty  &= ~MASK;\
pieces |= empty & (pieces OP V1);\
empty  &=         (empty  OP V1);\
pieces |= empty & (pieces OP V2);\
empty  &=         (empty  OP V2);\
pieces |= empty & (pieces OP V3);\
return    ~MASK & (pieces OP V1);
//test
u64 board_n_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(<<, 0, 8, 16, 32);
}
//test
u64 board_s_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(>>, 0, 8, 16, 32);
}
//test
u64 board_e_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(<<, FILE_A, 1, 2, 4);
}
//test
u64 board_w_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(>>, FILE_H, 1, 2, 4);
}
//test
u64 board_ne_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(<<, FILE_A, 9, 18, 36);
}
//test
u64 board_se_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(>>, FILE_A, 7, 14, 28);
}
//test
u64 board_nw_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(<<, FILE_H, 7, 14, 28);
}
//test
u64 board_sw_attacks(u64 pieces, u64 empty){
	KOGGE_STONE(>>, FILE_H, 9, 18, 36);
}

u8 board_count_bits(u64 x){
	u8 result;
	for(result = 0; x; result++)
		x &= x - 1;
	return result;
}

//test
//pos should reset to zero after visiting each bit
u64 iterate_bits(u64 x, u64 pos){
	u64 mask, alt;
	mask = ~(pos | (pos-1));
	alt = mask & x;
	return alt & -alt;
}

int board_list(Board* board, u8 side, BoardMoveNode* head){
//FOR SLIDERS
//* get piece mask
//** board->pieces[SLIDER]
//* get empty mask
//** get all piece mask
//** invert all piece mask
//* slider_attacks(piece mask, empty mask)
	u64 length;
	u64 piece;
	u64 bboard;

	length = 0;
	piece  = NULL;
	#define FOR_SLIDER(P, T) \
		for(P = iterate_bits(T, P);\
			P; P = iterate_bits(T, P))
//bishops
	FOR_SLIDER(piece, board->pieces[side][BISHOP]){
		//do stuff
		bboard = 
		++length;
	}
//rooks
//queens

}

int board_play(Board* board, BoardMoveNode* move){
//	board->pieces[move->beg_piece] &= ~(move->beg);
//	board->pieces[move->end_piece] |=  (move->end);
}

