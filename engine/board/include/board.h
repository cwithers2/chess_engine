#ifndef BOARD_H
#define BOARD_H
#include <util.h>

struct Board{
	u64 pieces[SIDES][PIECES];
	u64 castle[SIDES];
	u8  active;
	u64 target;
	u16 halfmoves;
	u16 fullmoves;
};

struct BoardMove{
	BoardMove* next;
	u8 piece_type;
	u8 promotion;
	u64 from;
	u64 to;
};

int  board_init();
void board_destroy();

void board_new(Board* board, char* fen);
void board_copy(Board* copy, Board* original);
void board_play(Board* board, BoardMove* move);
int  board_moves(Board* board, BoardMove** tail);
void board_moves_free(BoardMove* head);
#endif
