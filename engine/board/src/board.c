#include <board.h>
#include <string.h>
#include <stdlib.h>

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

struct Board{
	u64  pieces[SIDES][PIECES];
	char castle[SIDES][2];
	char active;
	char target[2];
	u16  halfmoves;
	u16  fullmoves;
};

struct BoardMove{
	int piece;
	int promotion;
	u64 from;
	u64 to;
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

