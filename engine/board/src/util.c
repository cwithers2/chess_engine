#include <stdlib.h>
#include <string.h>
#include <util.h>
#include <board.h>

const u64 BIT_RANKS[8] = {
#define X(RANK, BITS, CHAR) BITS,
#include <x/rank.h>
#undef X
};

const u64 BIT_FILES[8] = {
#define X(FILE, BITS, CHAR) BITS,
#include <x/file.h>
#undef X
};

const char* SQ_NAMES[64] = {
	"h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
	"h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
	"h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
	"h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
	"h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
	"h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
	"h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
	"h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8"
};

u64 board_rand64(){
	u64 result;
	result |= random() & 0xFFFF; result <<= 16;
	result |= random() & 0xFFFF; result <<= 16;
	result |= random() & 0xFFFF; result <<= 16;
	result |= random() & 0xFFFF;
	return result;
}

u64 board_lprand64(){
	return board_rand64() & board_rand64() & board_rand64() & board_rand64();
}

int board_ctz64(u64 value){
#ifdef __GNUC__
	return __builtin_ctzl(value);
#else
#error Function not defined for this compiler.
#endif
}

int board_pop64(u64 value){
#ifdef __GNUC__
	return __builtin_popcountl(value);
#else
#error Function not defined for this compiler.
#endif
}

u64 board_get_pos(const char* str){
	u64 result;
	result = EMPTYSET;
	switch(str[0]){
	#define X(FILE, BITS, CHAR)\
	case CHAR:\
		result |= BITS;\
		break;
	#include <x/file.h>
	#undef X
	}
	switch(str[1]){
	#define X(RANK, BITS, CHAR)\
	case CHAR:\
		result |= BITS;\
		break;
	#include <x/rank.h>
	#undef X
	}
	return result;
}

u64  board_get_rank(const u64 piece){
	return BIT_RANKS[board_ctz64(piece) / 8];
}

u64  board_get_file(const u64 piece){
	return BIT_FILES[board_ctz64(piece) % 8];
}

void board_format_pos(const u64 pos, char* str){
	strcpy(str, SQ_NAMES[board_ctz64(pos)]);
}

void board_format_move(const BoardMove* move, char* str){
	board_format_pos(move->from, str);
	board_format_pos(move->to, str+2);
	if(move->piece_type == move->promotion)
		return;
	if(move->piece_type != PAWN)
		return;
	switch(move->promotion){
	#define X(SIDE, PIECE, CHAR)\
	case PIECE:\
		str[4] = CHAR;\
		break;
	#include <x/black.h>
	#undef X
	}
	str[5] = '\0';
}

BoardMove* board_find_move(const BoardMove* moves, const char* str){
	BoardMove const* ptr;
	char formatted[6];
	ptr = moves;
	while(ptr){
		board_format_move(ptr, formatted);
		if(strcmp(formatted, str) == 0)
			return (BoardMove*)ptr;
		ptr = ptr->next;
	}
	return NULL;
}
