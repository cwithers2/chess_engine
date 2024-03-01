#ifndef TEST_H
#define TEST_H

/* USAGE:
 * Define ALL of your test functions with signature test_func (see below).
 * Call the TEST_GENERATE(...) macro and pass all test_funcs with comma seperation.
 * This will generate a main() function for compilation.
 *
 * NOTE:
 * test_func() should return !0 on success and 0 on failure.
 *
 * NOTE:
 * This file defines main(...);
 */

#include <stdio.h>
#include <time.h>

#define FAIL "\033[0;31mFAIL\033[0m"
#define PASS "\033[0;32mPASS\033[0m"
#define HEAD "\033[0;35m%s:\n\033[0m"

typedef int(*test_func)(void);
int TEST_PASS;
char* TEST_NAME;
#define TEST_RUN(IDX)\
do{\
	double delta;\
	clock_t t;\
	delta = clock();\
	TEST_PASS = test_cases[i-1]();\
	delta = ((double)(clock() - delta))/CLOCKS_PER_SEC;\
	printf(HEAD, TEST_NAME);\
	printf("\tTIME: %f seconds\n", delta);\
	if(!TEST_PASS) printf("\tSTATUS: %s\n", FAIL);\
	else printf("\tSTATUS: %s\n", PASS);\
}while(0)

#define TEST_GENERATE(...)\
test_func test_cases[] = {__VA_ARGS__};\
int main(){\
	size_t i, count;\
	int result = 0;\
	count = sizeof(test_cases)/sizeof(test_func);\
	for(i = 1; i <= count; ++i){\
		TEST_RUN(i);\
		if(!TEST_PASS)\
			result = i;\
	}\
	return result;\
}
#endif

