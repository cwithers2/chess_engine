#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <board.h>

#define INPUT(S, N) fgets(S, N, stdin)
#define DONE   40
#define NOMOVE 41
#define EXIT(EXP) exit(strcmp(EXP, EXPECT) != 0)

char EXPECT[20];
void setup_new_board(Board* board){
	char fen[120];
	INPUT(fen, 120);
	if(fen[0] == '\n')
		board_new(board, NULL);
	else
		board_new(board, fen);
}

void play_move(Board* board, BoardMove* moves){
	char move[10];
	BoardMove* lookup;
	if(INPUT(move, 10) == NULL)
		if(feof(stdin))
			EXIT("DONE");
		else
			EXIT("FILE ERROR");
	move[strcspn(move, "\n")] = 0;
	lookup = board_find_move(moves, move);
	if(!lookup)
		EXIT("BAD MOVE");
	board_play(board, lookup);
}

int main(int argc, char **argv){
	Board board;
	BoardMove head;
	INPUT(EXPECT, 20);
	EXPECT[strcspn(EXPECT, "\n")] = 0;
	if(board_init(BOARD_MODE_STD) == BOARD_ERROR)
		EXIT("INITIALIZING ERROR");
	setup_new_board(&board);
	while(1){
		switch(board_moves(&board, &head)){
		case BOARD_ERROR:
			EXIT("GENERATING ERROR");
		case BOARD_CHECKMATE:
			EXIT("CHECKMATE");
		case BOARD_STALEMATE:
			EXIT("STALEMATE");
		default:
			break;
		}
		play_move(&board, head.next);
		board_moves_free(head.next);
	}
}
