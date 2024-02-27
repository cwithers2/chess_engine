#ifndef BOARD_TYPES_H
#define BOARD_TYPES_H

typedef struct Board Board;
typedef union BoardMove BoardMove;

typedef enum{
	BOARD_WHITE,
	BOARD_BLACK
}BoardColor;

typedef enum{ // ASCII '1' - '8'
	BOARD_RANK_1 = 0x31,
	BOARD_RANK_2 = 0x32,
	BOARD_RANK_3 = 0x33,
	BOARD_RANK_4 = 0x34,
	BOARD_RANK_5 = 0x35,
	BOARD_RANK_6 = 0x36,
	BOARD_RANK_7 = 0x37,
	BOARD_RANK_8 = 0x38
}BoardRank;

typedef enum{ // ASCII 'a' - 'h'
	BOARD_FILE_A = 0x61,
	BOARD_FILE_B = 0x62,
	BOARD_FILE_C = 0x63,
	BOARD_FILE_D = 0x64,
	BOARD_FILE_E = 0x65,
	BOARD_FILE_F = 0x66,
	BOARD_FILE_G = 0x67,
	BOARD_FILE_H = 0x68
}BoardFile;

typedef enum{ // ASCII NULL, 'n', 'b', 'r', 'q'
	BOARD_PROMOTE_NULL   = 0x00,
	BOARD_PROMOTE_KNIGHT = 0x6e,
	BOARD_PROMOTE_BISHOP = 0x62,
	BOARD_PROMOTE_ROOK   = 0x72,
	BOARD_PROMOTE_QUEEN  = 0x71
}BoardPromote;

union BoardMove{
	char ascii[6];
	struct{
		char file_start;
		char rank_start;
		char file_end;
		char rank_end;
		char promotion;
	};
};

#endif
