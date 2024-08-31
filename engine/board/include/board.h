#ifndef BOARD_H
#define BOARD_H
#include <board_types.h>

int board_new(Board* board, const char* fen);
int board_copy(Board* copy, const Board* original);
int board_list(Board* board, u8 side, BoardMoveNode* head);
int board_play(Board* board, BoardMoveNode* move);

#endif
