#ifndef BOARD_H
#define BOARD_H
#include <util.h>

enum BOARD_MODE{
	BOARD_MODE_STD,
	BOARD_MODE_960
};

enum STATUS{
	BOARD_ERROR        = 0,
	BOARD_SUCCESS      = 1,
};

enum STATE{
	BOARD_NO_CHECK     = 0x00,
	BOARD_STALEMATE    = 0x01,
	BOARD_CHECK        = 0x02,
	BOARD_DOUBLE_CHECK = 0x06,
	BOARD_CHECKMATE    = BOARD_CHECK | BOARD_STALEMATE
};

struct Board{
	u64 pieces[SIDES][PIECES];
	u64 castle[SIDES];
	u8  active;
	u64 target;
	u16 halfmoves;
	u16 fullmoves;
	int state;
};

struct BoardMove{
	BoardMove* next;
	u8 piece_type;
	u8 promotion;
	u64 from;
	u64 to;
};
/*
	Parameters
	----------
	int mode:
		Which mode to initialize the library in. see enum BOARD_MODE.

	Returns
	-------
	int result:
		returns either BOARD_SUCCESS or BOARD_ERROR.
*/
int  board_init(int mode);
void board_destroy();

/*
	Initialize a board object from an FEN string.

	Parameters
	----------
	Board* board:
		The board to initialize.
	const char* fen:
		The FEN string to initialize from. If NULL, the default standard setup
		is used.

	NOTE: No validation is done for the FEN string.
*/
void board_new(Board* board, char* fen);

/*
	Copy one board to another.

	Parameters
	----------
	Board* copy:
		The board to copy to.
	Board* original:
		The board to copy from.
*/
void board_copy(Board* copy, Board* original);

/*
	Play a move on a board.

	Parameters
	----------
	Board* board:
		The board to play a move on.
	BoardMove* move:
		The move to play.
*/
void board_play(Board* board, BoardMove* move);

/*
	Get a list of current legal moves.

	Parameters
	----------
	Board* board:
		The board to generate moves for.
	BoardMove* head:
		The head node of the list to generate.

	NOTE: The head node will not have move data, but head->next will.
	NOTE: The generated moves will need to be freed with board_moves_free(...).

	Returns
	-------
	int result:
		Returns BOARD_ERROR or BOARD_SUCCESS.
*/
int  board_moves(Board* board, BoardMove* head);

/*
	Free generated moves.

	Parameters
	----------
	BoardMove* moves:
		The moves to free.
*/
void board_moves_free(BoardMove* moves);
#endif
