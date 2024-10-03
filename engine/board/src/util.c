#include <stdlib.h>
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
	#define X(FILE, BITS, CHAR)\
	if(pos & BITS) str[0] = CHAR;
	#include <x/file.h>
	#undef X
	#define X(RANK, BITS, CHAR)\
	if(pos & BITS) str[1] = CHAR;
	#include <x/rank.h>
	#undef X
	str[2] = '\0';
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
