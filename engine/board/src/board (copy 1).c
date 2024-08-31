#include <board.h>
#include <debug.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

typedef uint64_t BoardBits;

#define FILE_A         'a'
#define FILE_B         'b'
#define FILE_C         'c'
#define FILE_D         'd'
#define FILE_E         'e'
#define FILE_F         'f'
#define FILE_G         'g'
#define FILE_H         'h'

#define RANK_1         '1'
#define RANK_2         '2'
#define RANK_3         '3'
#define RANK_4         '4'
#define RANK_5         '5'
#define RANK_6         '6'
#define RANK_7         '7'
#define RANK_8         '8'

#define PROMOTE_KNIGHT 'n'
#define PROMOTE_BISHOP 'b'
#define PROMOTE_ROOK   'r'
#define PROMOTE_QUEEN  'q'

#define WHITE_PAWN     'P'
#define WHITE_KNIGHT   'N'
#define WHITE_BISHOP   'B'
#define WHITE_ROOK     'R'
#define WHITE_QUEEN    'Q'
#define WHITE_KING     'K'

#define BLACK_PAWN     'p'
#define BLACK_KNIGHT   'n'
#define BLACK_BISHOP   'b'
#define BLACK_ROOK     'r'
#define BLACK_QUEEN    'q'
#define BLACK_KING     'k'

#define PLAYER_WHITE   'w'
#define PLAYER_BLACK   'b'

#define FEN_DASH       '-'

typedef union BoardCoord BoardCoord;

union BoardCoord{
	char data[2];
	struct{
		char file;
		char rank;
	};
};

struct Board{
	struct{
		BoardBits pawns;
		BoardBits knights;
		BoardBits bishops;
		BoardBits rooks;
		BoardBits queens;
		BoardBits kings;
		char castle[2];
	}white, black;
	char active;
	BoardCoord target;
	unsigned int halfmoves; // Number of halfmoves since the last capture/pawn advance
	unsigned int fullmoves; // Number of full moves; Starts at 1 and counts after black
};

union BoardMove{
	char text[6];
	struct{
		BoardCoord beg;
		BoardCoord end;
		char promotion;
	};
};

#ifndef NO_DEBUG
void debug_print_board(Board* board){
	char buffer[64+9];
	int row, col, index = 0;
	BoardBits focus = 0x8000000000000000;
	debug_print("%s", "Printing board data.");
	debug_print("%ld", board->white.rooks);
	for(row = 0; row < 8; ++row){
		for(col = 0; col < 8; ++col){
			if(focus & board->white.pawns)
				buffer[index++] = WHITE_PAWN;
			else if(focus & board->white.knights)
				buffer[index++] = WHITE_KNIGHT;
			else if(focus & board->white.bishops)
				buffer[index++] = WHITE_BISHOP;
			else if(focus & board->white.rooks)
				buffer[index++] = WHITE_ROOK;
			else if(focus & board->white.queens)
				buffer[index++] = WHITE_QUEEN;
			else if(focus & board->white.kings)
				buffer[index++] = WHITE_KING;
			else if(focus & board->black.pawns)
				buffer[index++] = BLACK_PAWN;
			else if(focus & board->black.knights)
				buffer[index++] = BLACK_KNIGHT;
			else if(focus & board->black.bishops)
				buffer[index++] = BLACK_BISHOP;
			else if(focus & board->black.rooks)
				buffer[index++] = BLACK_ROOK;
			else if(focus & board->black.queens)
				buffer[index++] = BLACK_QUEEN;
			else if(focus & board->black.kings)
				buffer[index++] = BLACK_KING;
			else
				buffer[index++] = '.';
			focus >>= 1;
		}
		buffer[index++] = '\n';
	}
	buffer[index] = '\0';
	debug_print("%s", buffer);
}
#endif

int board_fen_pieces(Board* board, char* pieces){
	char* ptr = pieces;
	int slash = 7;
	BoardBits focus = 0x8000000000000000;
	debug_print("%s", "Parsing FEN string pieces.");
	debug_print("FEN string pieces: %s", pieces);
	while(*ptr){
		switch(*ptr){
		case WHITE_PAWN:
			board->white.pawns   |= focus;
			break;
		case WHITE_KNIGHT:
			board->white.knights |= focus;
			break;
		case WHITE_BISHOP:
			board->white.bishops |= focus;
			break;
		case WHITE_ROOK:
			board->white.rooks   |= focus;
			break;
		case WHITE_QUEEN:
			board->white.queens  |= focus;
			break;
		case WHITE_KING:
			board->white.kings   |= focus;
			break;
		case BLACK_PAWN:
			board->black.pawns   |= focus;
			break;
		case BLACK_KNIGHT:
			board->black.knights |= focus;
			break;
		case BLACK_BISHOP:
			board->black.bishops |= focus;
			break;
		case BLACK_ROOK:
			board->black.rooks   |= focus;
			break;
		case BLACK_QUEEN:
			board->black.queens  |= focus;
			break;
		case BLACK_KING:
			board->black.kings   |= focus;
			break;
		case '8':
			focus >>= 1;
		case '7':
			focus >>= 1;
		case '6':
			focus >>= 1;
		case '5':
			focus >>= 1;
		case '4':
			focus >>= 1;
		case '3':
			focus >>= 1;
		case '2':
			focus >>= 1;
		case '1':
			break;
		case '/':
			--slash;
			++ptr;
			continue;
		default:
			//error
			goto ABORT;
		}
		focus >>= 1;
		++ptr;
	}
	if(slash){
		//error
		goto ABORT;
	}
	if(focus){
		//error
		goto ABORT;
	}
	return 1;
	ABORT:
	free(ptr);
	return 0;
}

int board_fen_active(Board* board, const char active){
	switch(active){
	case PLAYER_WHITE:
		board->active = PLAYER_WHITE;
		break;
	case PLAYER_BLACK:
		board->active = PLAYER_BLACK;
		break;
	default:
		//error
		goto ABORT;
	}
	return 1;
	ABORT:
	return 0;
}

int board_fen_castle(Board* board, char* castle){
	size_t len = strlen(castle);
	char* ptr;
	if(len == 1 && *castle == FEN_DASH)
		return 1;
	if(len > 4){
		//error
		goto ABORT;
	}
	ptr = castle;
	while(*ptr){
		switch(*ptr){
		case WHITE_KING:
			board->white.castle[0] = WHITE_KING;
			break;
		case WHITE_QUEEN:
			board->white.castle[1] = WHITE_QUEEN;
			break;
		case BLACK_KING:
			board->black.castle[0] = BLACK_KING;
			break;
		case BLACK_QUEEN:
			board->black.castle[1] = BLACK_QUEEN;
			break;
		default:
			//error
			goto ABORT;
		}
		++ptr;
	}
	return 1;
	ABORT:
	return 0;
}

int board_fen_target(Board* board, const char* target){
	size_t len = strlen(target);
	if(len == 1 && *target == FEN_DASH)
		return 1;
	if(len != 2){
		debug_print("Invalid target square: %s", target);
		goto ABORT;
	}
	if((target[0] <  FILE_A)||(target[0] >  FILE_H)){
		//error
		goto ABORT;
	}
	if((target[1] != RANK_3)||(target[1] != RANK_7)){
		//error
		goto ABORT;
	}
	memcpy(board->target.data, target, 2);
	return 1;
	ABORT:
	return 0;
}

//test
int board_new(Board* board, char* fen){
	int result;
	char* ptr;
	char pieces[72];
	char active;
	char castle[5];
	char target[3];
	unsigned int halfmoves;
	unsigned int fullmoves;
	board = (Board*)calloc(1, sizeof(Board));
	if(!board){
		//error
		debug_print("%s", "Failed to allocate new board.");
		goto ABORT;
	}
	debug_print("%ld", board->white.rooks);
	if(!fen)
		ptr = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	else
		ptr = fen;
	result = sscanf(ptr, "%s %c %s %s %u %u",
	                pieces, &active, castle, target, &halfmoves, &fullmoves);
	if(result != 6){
		debug_print("%s", "Too few arguments read from FEN string.");
		goto ABORT;
	}
	if(!board_fen_pieces(board, pieces)){
		debug_print("%s", "Invalid piece data in FEN string.");
		goto ABORT;
	}
	if(!board_fen_active(board, active)){
		debug_print("%s", "Invalid active player in FEN string.");
		goto ABORT;
	}
	if(!board_fen_castle(board, castle)){
		debug_print("%s", "Invalid castle pieces in FEN string.");
		goto ABORT;
	}
	if(!board_fen_target(board, target)){
		debug_print("%s", "Invalid target square in FEN string.");
		goto ABORT;
	}
	board->halfmoves = halfmoves;
	board->fullmoves = fullmoves;
	debug_print_board(board);
	return 1;
	ABORT:
	free(board);
	return 0;
}

//needed?
int board_copy(Board* board, Board* copy){
}

int board_list(Board* board, int* argc, BoardMove** argv){
}

int board_play(Board* board, BoardMove* move){
}

void board_free_moves(int argc, BoardMove* argv){
}
