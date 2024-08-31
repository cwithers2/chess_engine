#include <stdio.h>
#include "include/board.h"

int main(int argc, char **argv){
	char fen[] = "1n6/2BrPPPp/1R1bp1k1/2QppNr1/1pn1b3/1p1PPppP/2P1BRP1/1qNK4 w kq - 0 1";
    Board* board = board_new(fen);
    if(!board)
    	return 1;
    return 0;
}
