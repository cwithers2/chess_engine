#include <stdio.h>
#include <stdint.h>

#define U64 uint64_t
#define u64 uint64_t

U64 afile = 0x8080808080808080;
U64 hfile = 0x0101010101010101;

int white = 0;
int black = 0;

u64 prom[] = {
	0x00000000000000FF,
	0xFF00000000000000
};

u64 home[] = {
	0x00FF000000000000,
	0x000000000000FF00
};

u64 wadv(u64 piece, int shift){
	return piece << shift;
}

u64 badv(u64 piece, int shift){
	return piece >> shift;
}

u64 (*adv[2])(u64, int) = {
	wadv,
	badv
}
#define WHITE 0
#define BLACK 1

u64 pawn_advance(u64 piece, int side){
	switch(side){
	case WHITE:
		return piece << 1;
	case BLACK:
		return piece >> 1;
	};
}
U64 pawn_lookup(u64 piece, u64 bboard, int side){
	u64 result, attack;
	u64 prom[] = {0x00000000000000FF, 0xFF00000000000000};
	u64 home[] = {0x00FF000000000000, 0x000000000000FF00};
	result = 0ULL;
	//ignore promotion squares
	if(piece & prom[side])
		return result;
	//single and double advance
	if(!(pawn_advance(piece, side) & bboard)){
		result |= pawn_advance(piece, side);
		if(piece & home[side] && !(pawn_advance(result, side) & bboard))
			result |= pawn_advance(result, side);
	}
	//diagonal left
	if(!(piece & FILE_A)){
		attack = pawn_advance(piece << 1, side);
		if(attack & bboard)
			result |= attack;
	}
	//diagonal right
	if(!(piece & FILE_H)){
		attack = pawn_advance(piece >> 1, side);
		if(attack & bboard)
			result |= attack;
	}
	return result;
}

// Function to generate bitboard for white pawn moves
U64 generate_white_pawn_moves(int square) {
	U64 bitboard = 0ULL;
	U64 one = 1ULL;
	U64 pawn = one << square;
	//ignore invalid positions
	if(square >= 56)
		return 0ULL;

	// Single move forward
	bitboard |= (pawn << 8);

	// Double move forward from the second rank
	if(square > 7 && square < 16)
		bitboard |= (pawn << 16);

	// Capture diagonally to the left
	if (square % 8 != 0) 
		bitboard |= (pawn << 7);

	// Capture diagonally to the right
	if (square % 8 != 7)
		bitboard |= (pawn << 9);

	return bitboard;
}


U64 generate_black_pawn_moves(int square) {
	U64 bitboard = 0ULL;
	U64 one = 1ULL;
	U64 pawn = one << square;
	//ignore invalid positions
	if(square < 8)
		return 0ULL;

	// Single move forward
	bitboard |= (pawn >> 8);

	// Double move forward from the second rank
	if(square > 48 && square < 56)
		bitboard |= (pawn << 16);

	// Capture diagonally to the left
	if (square % 8 != 0) 
		bitboard |= (pawn << 7);

	// Capture diagonally to the right
	if (square % 8 != 7)
		bitboard |= (pawn << 9);

	return bitboard;
}

int main() {
    U64 white_pawn_moves[64];

    for (int i = 0; i < 64; i++) {
        white_pawn_moves[i] = generate_white_pawn_moves(i);
    }

    // Print the bitboards for verification
    printf("u64 wptable[64] = {\n");
    for (int i = 0; i < 64; i++) {
        printf("\t0x%016lxULL,\n", white_pawn_moves[i]);
    }
    printf("};\n");

    return 0;
}

