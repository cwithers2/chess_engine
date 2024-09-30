#include <stdio.h>
#include "include/board.h"
#include "include/debug.h"

int main(int argc, char **argv){
	char fen[120];
	char loc[3];
	char move[6];
	Board board;
	BoardMove data;
	BoardMove *head = &data, *iter;
	printf("Initializing board library. Please wait.\n");
	if(!board_init()){
		printf("Failed to initialize board library.\n");
		return 1;
	}
	printf("Enter FEN> ");
	fgets(fen, 120, stdin);
	board_new(&board, fen);
	printf("Enter an active piece's location> ");
	fgets(loc, 3, stdin);
	printf("Generating board moves.\n");
	if(!board_moves(&board, &head)){
		printf("failed to generate board moves.\n");
		return 2;
	}
	printf("Moves: ");
	iter = data.next;
	while(iter){
		board_format_move(iter, move);
		printf("%s ", move);
		iter = iter->next;
	}
	printf("\n");
	return 0;
}
