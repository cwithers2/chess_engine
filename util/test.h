#ifndef TEST_H
#define TEST_H

/* USAGE:
 * Define ALL of your test functions with signature test_func (see below).
 * Call the TEST_GENERATE(...) macro and pass all test_funcs with comma seperation.
 * This will generate a main() function for compilation. Define TEST_NO_MAIN to disable.
 * Check int TEST_PASS for test results if TEST_NO_MAIN is defined.
 *
 * NOTE:
 * test_func() should return !0 on success and 0 on failure.
 *
 * NOTE:
 * This file defines main(...); Disable by defining TEST_NO_MAIN
 */

#include <stdio.h>
#include <time.h>

#define FAIL "\033[0;31mFAIL\033[0m"
#define PASS "\033[0;32mPASS\033[0m"
#define HEAD "\033[0;35mRUNNING TEST #%ld:\n\033[0m"

typedef int(*test_func)(void);
int TEST_PASS;

#define TEST_RUN(IDX)\
do{\
	double delta;\
	clock_t t;\
	printf(HEAD, IDX+1);\
	delta = clock();\
	TEST_PASS = test_cases[i]();\
	delta = ((double)(clock() - delta))/CLOCKS_PER_SEC;\
	printf("\tTIME: %f seconds\n", delta);\
	if(!TEST_PASS) printf("\tSTATUS: %s\n", FAIL);\
	else printf("\tSTATUS: %s\n", PASS);\
}while(0)

#ifdef TEST_NO_MAIN
#define TEST_GENERATE(...)\
test_func test_cases[] = {__VA_ARGS__};\
do{\
	size_t i, count;\
	count = sizeof(test_cases)/sizeof(test_func);\
	for(i = 0; i < count; ++i){\
		TEST_RUN(i);\
		if(!TEST_PASS)\
			break;\
	}\
}while(0)
#else
#define TEST_GENERATE(...)\
test_func test_cases[] = {__VA_ARGS__};\
int main(){\
	size_t i, count;\
	count = sizeof(test_cases)/sizeof(test_func);\
	for(i = 0; i < count; ++i){\
		TEST_RUN(i);\
		if(!TEST_PASS)\
			break;\
	}\
}
#endif

#endif
