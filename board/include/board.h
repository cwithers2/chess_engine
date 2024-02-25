#ifndef BOARD_H
#define BOARD_H
#include <board_types.h>

Board* board_init();
void board_destroy(Board* board);
int board_set(Board* board, char* fen);
int board_copy(Board* src, Board* dest);
int board_list(Board* board, BoardColor color, int* count, BoardMove** moves);
int board_move(BoardMove* move);

#endif
