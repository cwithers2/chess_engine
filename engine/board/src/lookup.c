#include <stdlib.h>
#include <string.h>
#include <lookup.h>
#include <debug.h>

//forward declare lookup functions
#define LOOKUP_FUNC(TYPE) lookup ## TYPE
#define X(TYPE)           static u64 LOOKUP_FUNC(TYPE)(u64, u64, int);
#include <x/piece.h>
#undef X

//create dispatch table
typedef u64 (*lookup_func_t)(u64, u64, int);
const lookup_func_t lookup_dispatch[] = {
	#define X(TYPE) LOOKUP_FUNC(TYPE),
	#include <x/piece.h>
	#undef X
};

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

const u64 ktable[64] = {
	0x0000000000000302ULL, 0x0000000000000705ULL,
	0x0000000000000E0AULL, 0x0000000000001C14ULL,
	0x0000000000003828ULL, 0x0000000000007050ULL,
	0x000000000000E0A0ULL, 0x000000000000C040ULL,
	0x0000000000030203ULL, 0x0000000000070507ULL,
	0x00000000000E0A0EULL, 0x00000000001C141CULL,
	0x0000000000382838ULL, 0x0000000000705070ULL,
	0x0000000000E0A0E0ULL, 0x0000000000C040C0ULL,
	0x0000000003020300ULL, 0x0000000007050700ULL,
	0x000000000E0A0E00ULL, 0x000000001C141C00ULL,
	0x0000000038283800ULL, 0x0000000070507000ULL,
	0x00000000E0A0E000ULL, 0x00000000C040C000ULL,
	0x0000000302030000ULL, 0x0000000705070000ULL,
	0x0000000E0A0E0000ULL, 0x0000001C141C0000ULL,
	0x0000003828380000ULL, 0x0000007050700000ULL,
	0x000000E0A0E00000ULL, 0x000000C040C00000ULL,
	0x0000030203000000ULL, 0x0000070507000000ULL,
	0x00000E0A0E000000ULL, 0x00001C141C000000ULL,
	0x0000382838000000ULL, 0x0000705070000000ULL,
	0x0000E0A0E0000000ULL, 0x0000C040C0000000ULL,
	0x0003020300000000ULL, 0x0007050700000000ULL,
	0x000E0A0E00000000ULL, 0x001C141C00000000ULL,
	0x0038283800000000ULL, 0x0070507000000000ULL,
	0x00E0A0E000000000ULL, 0x00C040C000000000ULL,
	0x0302030000000000ULL, 0x0705070000000000ULL,
	0x0E0A0E0000000000ULL, 0x1C141C0000000000ULL,
	0x3828380000000000ULL, 0x7050700000000000ULL,
	0xE0A0E00000000000ULL, 0xC040C00000000000ULL,
	0x0203000000000000ULL, 0x0507000000000000ULL,
	0x0A0E000000000000ULL, 0x141C000000000000ULL,
	0x2838000000000000ULL, 0x5070000000000000ULL,
	0xA0E0000000000000ULL, 0x40C0000000000000ULL
};

const u64 ntable[64] = {
	0x0000000000020400ULL, 0x0000000000050800ULL,
	0x00000000000A1100ULL, 0x0000000000142200ULL,
	0x0000000000284400ULL, 0x0000000000508800ULL,
	0x0000000000A01000ULL, 0x0000000000402000ULL,
	0x0000000002040004ULL, 0x0000000005080008ULL,
	0x000000000A110011ULL, 0x0000000014220022ULL,
	0x0000000028440044ULL, 0x0000000050880088ULL,
	0x00000000A0100010ULL, 0x0000000040200020ULL,
	0x0000000204000402ULL, 0x0000000508000805ULL,
	0x0000000A1100110AULL, 0x0000001422002214ULL,
	0x0000002844004428ULL, 0x0000005088008850ULL,
	0x000000A0100010A0ULL, 0x0000004020002040ULL,
	0x0000020400040200ULL, 0x0000050800080500ULL,
	0x00000A1100110A00ULL, 0x0000142200221400ULL,
	0x0000284400442800ULL, 0x0000508800885000ULL,
	0x0000A0100010A000ULL, 0x0000402000204000ULL,
	0x0002040004020000ULL, 0x0005080008050000ULL,
	0x000A1100110A0000ULL, 0x0014220022140000ULL,
	0x0028440044280000ULL, 0x0050880088500000ULL,
	0x00A0100010A00000ULL, 0x0040200020400000ULL,
	0x0204000402000000ULL, 0x0508000805000000ULL,
	0x0A1100110A000000ULL, 0x1422002214000000ULL,
	0x2844004428000000ULL, 0x5088008850000000ULL,
	0xA0100010A0000000ULL, 0x4020002040000000ULL,
	0x0400040200000000ULL, 0x0800080500000000ULL,
	0x1100110A00000000ULL, 0x2200221400000000ULL,
	0x4400442800000000ULL, 0x8800885000000000ULL,
	0x100010A000000000ULL, 0x2000204000000000ULL,
	0x0004020000000000ULL, 0x0008050000000000ULL,
	0x00110A0000000000ULL, 0x0022140000000000ULL,
	0x0044280000000000ULL, 0x0088500000000000ULL,
	0x0010A00000000000ULL, 0x0020400000000000ULL
};

const u8 rshift[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

const u8 bshift[64] = {
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

static int transform(u64 bboard, u64 magic, int bits){
	return (int)((bboard * magic) >> (64 - bits));
}

static u64 rmask(u64 piece){
	int i;
	u64 result;
	result = EMPTYSET;
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
	result = EMPTYSET;
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
	u64 result = EMPTYSET;
	debug_print("creating blocker %i for mask %lu", iteration, mask);
	while(iteration){
		if(iteration & 1){
			int shift = board_ctz64(mask);
			result |= 1ULL << shift;
		}
		iteration >>= 1;
		mask &= mask - 1ULL;
	}
	debug_print_bitboard(result);
	return result;
}

static u64 rattacks(int rank, int file, u64 block){
	u64 result = EMPTYSET;
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
	u64 result = EMPTYSET;
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
	u64 magic, hashed;
	int i, j, range, fail;
	range = 1 << shift;
	for(j = 0; j < 100000000; ++j){
		magic = board_lprand64();
		memset(table, 0, sizeof(u64) * range);
		fail = 0;
		for(i = 0; i < range; ++i){
			hashed = transform(blocks[i], magic, shift);
			if(table[hashed] == EMPTYSET)
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
int  board_lookup_init(){
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
	return BOARD_SUCCESS;
	ABORT:
	for(i = 0; i < 64; ++i){
		free(rtable[i]);
		free(btable[i]);
	}
	free(blocks);
	free(attacks);
	return BOARD_ERROR;
}

void board_lookup_destroy(){
	int i;
	for(i = 0; i < 64; ++i){
		free(rtable[i]);
		free(btable[i]);
	}
}

u64 board_lookup(int type, u64 piece, u64 bboard, int side){
	debug_print("Dispatching key: %i", type);
	return (*lookup_dispatch[type])(piece, bboard, side);
}

static u64 LOOKUP_FUNC(PAWN)(u64 piece, u64 bboard, int side){
	u64 attack, move, nboard;
	u8 attacks[] = { 0x02, 0x05, 0x0A, 0x14, 0x28, 0x50, 0xA0, 0x40 };
	int rank, file, index;
	index = board_ctz64(piece);
	rank = index/8;
	file = index%8;
	nboard = ~bboard;
	attack = attacks[file];
	switch(side){
	case WHITE:
		move   =  (piece << 8) & nboard;
		move  |=  (move  << 8) & RANK_4 & nboard;
		attack =  attack << (8*(rank+1));
		break;
	case BLACK:
		move   =  (piece >> 8) & nboard;
		move  |=  (move  >> 8) & RANK_5 & nboard;
		attack = attack << (8*(rank-1));
		break;
	}
	attack &= bboard;
	return move | attack;
}

static u64 LOOKUP_FUNC(KNIGHT)(u64 piece, u64 bboard, int side){
	return ntable[board_ctz64(piece)];
}

static u64 LOOKUP_FUNC(BISHOP)(u64 piece, u64 bboard, int side){
	u64 block;
	int index, hashed;
	index = board_ctz64(piece);
	block  = bmask(piece) & bboard;
	hashed = transform(block, bmagic[index], bshift[index]);
	return btable[index][hashed];
}

static u64 LOOKUP_FUNC(ROOK)(u64 piece, u64 bboard, int side){
	u64 block;
	int index, hashed;
	index = board_ctz64(piece);
	block  = rmask(piece) & bboard;
	hashed = transform(block, rmagic[index], rshift[index]);
	return rtable[index][hashed];
}

static u64 LOOKUP_FUNC(QUEEN)(u64 piece, u64 bboard, int side){
	u64 block, temp;
	int index, hashed;
	index = board_ctz64(piece);
	block  = bmask(piece) & bboard;
	hashed = transform(block, bmagic[index], bshift[index]);
	temp   = btable[index][hashed];
	block  = rmask(piece) & bboard;
	hashed = transform(block, rmagic[index], rshift[index]);
	return temp | rtable[index][hashed];
}

static u64 LOOKUP_FUNC(KING)(u64 piece, u64 bboard, int side){
	return ktable[board_ctz64(piece)];
}

