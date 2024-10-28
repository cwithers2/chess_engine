#include <stdio.h>
#include <string.h>
#include "include/board.h"
#include "include/debug.h"
#include "include/evaluate.h"

void setup_new_board(Board*);
void list_moves(BoardMove*);
void play_move(Board*, BoardMove*);

void setup_new_board(Board* board){
	char fen[120];
	printf("Enter FEN> ");
	fgets(fen, 120, stdin);
	if(fen[0] == '\n')
		board_new(board, NULL);
	else
		board_new(board, fen);
}

void list_moves(BoardMove* moves){
	char move[6];
	BoardMove *iter;
	printf("Moves: ");
	iter = moves;
	while(iter){
		board_format_move(iter, move);
		printf("%s ", move);
		iter = iter->next;
	}
	printf("\n");
}

void play_move(Board* board, BoardMove* moves){
	char move[10];
	BoardMove* lookup;
	RETRY:
	printf("Enter move to play> ");
	fgets(move, 10, stdin);
	move[strcspn(move, "\n")] = 0;
	lookup = board_find_move(moves, move);
	if(!lookup){
		printf("Move not found.\n");
		goto RETRY;
	}
	board_play(board, lookup);
}

int main(int argc, char **argv){
	int status;
	int eval;
	Board board;
	BoardMove head;
	printf("Initializing board library. Please wait.\n");
	if(board_init(BOARD_MODE_STD) == BOARD_ERROR){
		fprintf(stderr, "Failed to initialize board library.\n");
		return 1;
	}
	setup_new_board(&board);
	while(1){
		debug_print_board(&board);
		eval = board_evaluate(&board);
		printf("EVAL:   %i\n", eval);
		status = board_moves(&board, &head);
		if(status == BOARD_ERROR){
			fprintf(stderr, "board moves returned non-zero: %i", status);
			return status;
		}
		list_moves(head.next);
		play_move(&board, head.next);
		board_moves_free(head.next);
	}
}
