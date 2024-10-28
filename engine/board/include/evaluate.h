#ifndef EVALUATE_H
#define EVALUATE_H

#include <limits.h> // INT_MAX and INT_MIN
#include "board.h"

/*
	Returns an integer evaluation for the current board position.
	
	Returns
	-------
	0 -> stalemate.
	INT_MAX -> BLACK is in checkmate.
	INT_MIN -> WHITE is in checkmate.
	+int -> WHITE has advantage.
	-int -> BLACK has advantage.
*/
int board_evaluate(Board* board);
#endif
