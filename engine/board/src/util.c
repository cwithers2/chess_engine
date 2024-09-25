#include <stdlib.h>
#include <util.h>
#include <board.h>

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
	u8 result;
	if(!value)
		return 64;
	value &= -value;
	result = 0;
	#define MASKED_ADD(MASK, ADD) if(value & MASK) result |= ADD
	MASKED_ADD(0xAAAAAAAAAAAAAAAA, 1);
	MASKED_ADD(0xCCCCCCCCCCCCCCCC, 2);
	MASKED_ADD(0xF0F0F0F0F0F0F0F0, 4);
	MASKED_ADD(0xFF00FF00FF00FF00, 8);
	MASKED_ADD(0xFFFF0000FFFF0000, 16);
	MASKED_ADD(0xFFFFFFFF00000000, 32);
	return result;
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
}

void board_format_move(const BoardMove* move, char* str){
	board_format_pos(move->from, str);
	board_format_pos(move->to, str+2);
}
