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
	u8 piece;
	u8 promotion;
	u64 from;
	u64 to;
};

struct BoardMoveNode{
	BoardMoveNode* next;
	BoardMove move;
};

int  board_init();
void board_destroy();

void board_new(Board* board, const char* fen);
void board_copy(Board* copy, const Board* original);
void board_play(Board* board, const BoardMove* move);
int  board_moves(const Board* board, BoardMoveNode* head);
void board_moves_free(BoardMoveNode* head);
#endif
