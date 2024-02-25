#include "test.h"

int test_pass(){
	return 1;
}

int test_fail(){
	return 0;
}

TEST_GENERATE(
	test_pass,
	test_fail,
	test_pass // never reached
);
