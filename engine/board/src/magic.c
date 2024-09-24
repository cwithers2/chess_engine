#include <stdlib.h>
#include <string.h>
#include <magic.h>
#include <board.h>
#include <debug.h>

u64 ranks[] = {
	#define X(RANK, BITS, CHAR) BITS,
	#include <x/rank.h>
	#undef X
};

u64 files[] = {
	#define X(FILE, BITS, CHAR) BITS,
	#include <x/file.h>
	#undef X
};

u64 diagonals[] = {
	#define X(BITS) BITS,
	#include <x/diagonal.h>
	#undef X
};

u8 rshift[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

u8 bshift[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

u64 rmagic[64];
u64 bmagic[64];

u64* rtable[64];
u64* btable[64];

static u16 transform(u64 bboard, u64 magic, u8 bits){
	return (u16)((bboard * magic) >> (64 - bits));
}

static u64 rand64(){
	u64 result;
	result |= random() & 0xFFFF; result <<= 16;
	result |= random() & 0xFFFF; result <<= 16;
	result |= random() & 0xFFFF; result <<= 16;
	result |= random() & 0xFFFF;
	return result;
}

static u64 lprand64(){
	return rand64() & rand64() & rand64() & rand64();
}

static u8 ctz64(u64 value){
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

static u64 rmask(u64 piece){
	int i;
	u64 result;
	result = BOARD_EMPTYSET;
	for(i = 0; i < 8; ++i)
		if(ranks[i] & piece){
			result |= ranks[i]&0x7e7e7e7e7e7e7e7e;
			break;
		}
	for(i = 0; i < 8; ++i)
		if(files[i] & piece){
			result |= files[i]&0x00FFFFFFFFFFFF00;
			break;
		}
	return result & ~piece;
}

static u64 bmask(u64 piece){
	int i;
	u64 result;
	result = BOARD_EMPTYSET;
	for(i = 0; i < 15; ++i)
		if(diagonals[i]&piece){
			result |= diagonals[i]&0x007e7e7e7e7e7e00;
			break;
		}
	for(i = 15; i < 30; ++i)
		if(diagonals[i]&piece){
			result |= diagonals[i]&0x007e7e7e7e7e7e00;
			break;
		}
	return result & ~piece;
}

static u64 blockers(u16 iteration, u64 mask){
	u64 result = BOARD_EMPTYSET;
	debug_print("creating blocker %i for mask %lu", iteration, mask);
	while(iteration){
		if(iteration & 1){
			u8 shift = ctz64(mask);
			result |= 1ULL << shift;
		}
		iteration >>= 1;
		mask &= mask - 1ULL;
	}
	debug_print_bitboard(result);
	return result;
}

static u64 rattacks(int rank, int file, u64 block){
	u64 result = BOARD_EMPTYSET;
	int r, f;
	debug_print("finding attacks for (%i,%i), %lu", rank, file, block);
	for(r = rank+1; r < 8; ++r){
		result |= ranks[r];
		if(block & ranks[r]) break;
	}
	for(r = rank-1; r >= 0; --r){
		result |= ranks[r];
		if(block & ranks[r]) break;
	}
	for(f = file+1; f < 8; ++f){
		result |= files[f];
		if(block & files[f]) break;
	}
	for(f = file-1; f >= 0; --f){
		result |= files[f];
		if(block & files[f]) break;
	}
	result &= ranks[rank] | files[file];
	debug_print_bitboard(result);
	return result;
}

static u64 battacks(int diag1, int diag2, u64 block){
	u64 result = BOARD_EMPTYSET;
	int d1, d2;
	debug_print("finding attacks for (%i,%i), %lu", diag1, diag2, block);
	for(d1 = diag1+1; d1 < 15; ++d1){
		result |= diagonals[d1];
		if(block & diagonals[d1]) break;
	}
	for(d1 = diag1-1; d1 >= 0; --d1){
		result |= diagonals[d1];
		if(block & diagonals[d1]) break;
	}
	for(d2 = diag2+1; d2 < 30; ++d2){
		result |= diagonals[d2];
		if(block & diagonals[d2]) break;
	}
	for(d2 = diag2-1; d2 >= 15; --d2){
		result |= diagonals[d2];
		if(block & diagonals[d2]) break;
	}
	result &= diagonals[diag1] | diagonals[diag2];
	debug_print_bitboard(result);
	return result;
}

static u64 find_magic(int shift, u64* blocks, u64* attacks, u64* table){
	u64 magic, hashed, used[4096];
	int i, j, range, fail;
	range = 1 << shift;
	for(j = 0; j < 100000000; ++j){
		magic = lprand64();
		memset(table, 0, sizeof(u64) * range);
		fail = 0;
		for(i = 0; i < range; ++i){
			hashed = transform(blocks[i], magic, shift);
			if(table[hashed] == BOARD_EMPTYSET)
				table[hashed] = attacks[i];
			else if(table[hashed] != attacks[i]){
				fail = 1;
				break;
			}
		}
		if(!fail)
			return magic;
	}
	return 0;
}

//public functions
int  magic_init(){
	u64 mask;
	int i, j;
	int rank, file, diag1, diag2;
	size_t size;
	u64 *blocks, *attacks;
	blocks  = malloc(sizeof(u64) * 4096);
	attacks = malloc(sizeof(u64) * 4096);
	debug_print("%s", "initializing rook magic.");
	//rooks
	for(i = 0; i < 64; ++i){
		rank = i/8;
		file = i%8;
		size = 1ULL << rshift[i];
		mask = rmask(1ULL<<i);
		debug_print_bitboard(ranks[rank]);
		debug_print_bitboard(files[file]);
		debug_print_bitboard(mask);
		debug_print("index: %i\nrank: %i\nfile: %i", i, rank, file);
		//allocate
		rtable[i] = malloc(sizeof(u64) * size);
		if(!rtable[i])
			goto ABORT;
		//populate
		debug_print("%s", "populating blockers and attackers.");
		for(j = 0; j < size; ++j){
			blocks[j]  = blockers(j, mask);
			attacks[j] = rattacks(rank, file, blocks[j]);
		}
		debug_print("%s", "populating rook magics.");
		rmagic[i] = find_magic(rshift[i], blocks, attacks, rtable[i]);
		if(!rmagic[i])
			goto ABORT;
	}
	debug_print("%s", "initializing bishop magic.");
	//bishops
	u64 bit;
	for(i = 0; i < 64; ++i){
		bit  = 1ULL << i;
		size = 1 << bshift[i];
		mask = bmask(bit);
		for(j = 0; j < 15; ++j)
			if(diagonals[j] & bit)
				diag1 = j;
		for(j = 15; j < 30; ++j)
			if(diagonals[j] & bit)
				diag2 = j;
		debug_print_bitboard(diagonals[diag1]);
		debug_print_bitboard(diagonals[diag2]);
		debug_print_bitboard(mask);
		//allocate
		btable[i] = malloc(sizeof(u64) * size);
		if(!btable[i])
			goto ABORT;
		//populate
		debug_print("%s", "populating blockers and attackers.");
		for(j = 0; j < size; ++j){
			blocks[j]  = blockers(j, mask);
			attacks[j] = battacks(diag1, diag2, blocks[j]);
		}
		debug_print("%s", "populating bishop magics.");
		bmagic[i] = find_magic(bshift[i], blocks, attacks, btable[i]);
		if(!bmagic[i])
			goto ABORT;
	}
	//cleanup
	free(blocks);
	free(attacks);
	return 1;
	ABORT:
	for(i = 0; i < 64; ++i){
		free(rtable[i]);
		free(btable[i]);
	}
	free(blocks);
	free(attacks);
	return 0;
}

void magic_destroy(){
	int i;
	for(i = 0; i < 64; ++i){
		free(rtable[i]);
		free(btable[i]);
	}
}

u64  magic_lookup(u64 piece, u64 bboard, int type){
	u64 hashed, block, result;
	int index;
	index = ctz64(piece);
	switch(type){
	case BISHOP:
		block  = bmask(piece) & bboard;
		hashed = transform(block, bmagic[index], bshift[index]);
		result = btable[index][hashed];
		break;
	case ROOK:
		block  = rmask(piece) & bboard;
		hashed = transform(block, rmagic[index], rshift[index]);
		result = rtable[index][hashed];
		break;
	}
	return result;
}
