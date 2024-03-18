#include <board.h>

struct Board{
};

union BoardMove{
};

int board_new(Board* board, const char* fen){
}

int board_copy(Board* board, Board* copy){
}

int board_list(Board* board, int* argc, BoardMove** argv){
}

int board_play(Board* board, BoardMove* move){
}

void board_free_moves(int argc, BoardMove* argv){
}
