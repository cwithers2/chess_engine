#include <stdio.h>
#include "include/board.h"

int main(int argc, char **argv){
	char fen[120];
	char move[6];
	int status;
	Board board;
	BoardMove head, *iter;
	printf("Initializing board library. Please wait.\n");
	if(board_init(BOARD_MODE_STD) == BOARD_ERROR){
		printf("Failed to initialize board library.\n");
		return 1;
	}
	printf("Enter FEN> ");
	fgets(fen, 120, stdin);
	board_new(&board, fen);
	printf("Generating board moves.\n");
	status = board_moves(&board, &head);
	printf("board_moves(...) returned %i\n", status);
	if(status == BOARD_ERROR){
		return 2;
	}
	printf("Moves: ");
	iter = head.next;
	while(iter){
		board_format_move(iter, move);
		printf("%s ", move);
		iter = iter->next;
	}
	printf("\n");
	board_moves_free(head.next);
	return 0;
}
