#ifndef DEBUG_H
#define DEBUG_H

//set flag NO_DEBUG to remove debug tokens from compliation.
#ifdef NO_DEBUG
#define debug_print(fmt, ...) ;
#else
#include <stdio.h>
#include <threads.h>
#include <time.h>
#define debug_print(fmt, ...) \
do{fprintf(stderr, fmt "\n\tFL:%s\n\tFN:%s()\n", \
__VA_ARGS__, __FILE__, __func__); }while(0)
#endif

#endif
