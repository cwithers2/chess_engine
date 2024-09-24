#ifndef DEBUG_H
#define DEBUG_H

//set flag DEBUG to include debug tokens.
#ifndef DEBUG
#define LOCAL_DECL ;
#define debug_print(fmt, ...) ;
#define debug_print_bitboard(B)  ;
#define debug_print_board(B) ;

#else
#define LOCAL_DECL static
#include <stdio.h>
#include <stdint.h>
#define debug_print(fmt, ...) \
do{fprintf(stderr, fmt "\n", __VA_ARGS__); }while(0)

#include "board.h"
void debug_print_bitboard(u64 bboard);
void debug_print_board(Board* board);

#endif
#endif
