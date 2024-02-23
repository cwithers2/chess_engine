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
do{fprintf(stderr,"THREAD:%li\n FL:%s\n LN:%d\n FN:%s()\n TM:%ld\n MG:" fmt, \
thrd_current(), __FILE__, __LINE__, __func__, time(0), __VA_ARGS__); }while(0)
#endif

#endif
