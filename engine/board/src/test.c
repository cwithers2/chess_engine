#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <board.h>

#define INPUT(S, N) fgets(S, N, stdin)
#define EXIT(EXP) do{\
	printf("End result: %s\n", EXP);\
	exit(strcmp(EXP, EXPECT) != 0);\
}while(0)

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
			switch(board->state){
			case BOARD_NO_CHECK:
				EXIT("NO CHECK");
			case BOARD_STALEMATE:
				EXIT("STALEMATE");
			case BOARD_CHECK:
				EXIT("CHECK");
			case BOARD_DOUBLE_CHECK:
				EXIT("DOUBLE CHECK");
			case BOARD_CHECKMATE:
				EXIT("CHECKMATE");
			default:
				EXIT("UNKNOWN STATE");
			}
		else
			EXIT("FILE ERROR");
	printf("Playing: %s", move);
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
	printf("Initializing library...\n");
	if(board_init(BOARD_MODE_STD) == BOARD_ERROR)
		EXIT("INITIALIZING ERROR");
	setup_new_board(&board);
	printf("Starting...\n");
	printf("Expected result: %s\n", EXPECT);
	while(1){
		if(board_moves(&board, &head) == BOARD_ERROR)
			EXIT("GENERATING ERROR");
		play_move(&board, head.next);
		board_moves_free(head.next);
	}
}
