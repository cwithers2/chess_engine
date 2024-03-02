#include <test.h>
#include <debug.h>

//tokenize tests
#include <tokenize.h>
int test1(){
	TEST_NAME = "test::tokenize::count_until_delim";
	char buffer[] = "Hello, World!";
	char delim[] = "j, t";
	return count_until_delim(buffer, delim) == 5;
}

int test2(){
	TEST_NAME = "test::tokenize::count_while_delim";
	char buffer[] = "Hello, World!";
	char delim[] = "HeloW ";
	return count_while_delim(buffer, delim) == 5;
}

int test3(){
	TEST_NAME = "test::tokenize::check_token";
	char buffer[] = "Hello, World!";
	char delim[] = ", ";
	char token[] = "Hello";
	return check_token(buffer, token, delim);
}

int test4(){
	TEST_NAME = "test::tokenize::check_token";
	char buffer[] = "Hello, World!";
	char delim[] = ", ";
	char token[] = "World!";
	return !check_token(buffer, token, delim);
}

int test5(){
	TEST_NAME = "test::tokenize::count_until_token";
	char buffer[] = "Hello, World!";
	char delim[] = ", ";
	char token[] = "Hello";
	return count_until_token(buffer, token, delim) == 0;
}

int test6(){
	TEST_NAME = "test::tokenize::count_until_token";
	char buffer[] = "Hello, World!";
	char delim[] = ", ";
	char token[] = "World";
	return count_until_token(buffer, token, delim) < 0;
}

int test7(){
	TEST_NAME = "test::tokenize::count_while_delim";
	char buffer[] = "  ,,  ,,, ,  Hello, World!";
	char delim[] = ", ";
	return count_while_delim(buffer, delim) == 13; 
}

int test8(){
	TEST_NAME = "test::tokenize::count_while_delim";
	char buffer[] = "Hello, World!";
	char delim[] = ", ";
	return count_while_delim(buffer, delim) == 0;
}

int test9(){
	TEST_NAME = "test::tokenize::count_until_delim";
	char buffer[] = " Hello, World!";
	char delim[] = ", ";
	return count_until_delim(buffer, delim) == 0;
}

int test10(){
	TEST_NAME = "test::tokenize::count_until_token";
	char buffer[] = "Hello, World!";
	char delim[] = ", !";
	char token[] = "World";
	return count_until_token(buffer, token, delim) == 7;
}

int test11(){
	TEST_NAME = "test::tokenize::count_until_token";
	char buffer[] = "Hello, World!";
	char delim[] = ", !";
	char token[] = "Hello";
	return count_until_token(buffer, token, delim) == 0;
}

int test12(){
	TEST_NAME = "test::tokenize::count_until_token";
	char buffer[] = "Hello, World!";
	char delim[] = ", !";
	char token[] = "Goodbye";
	return count_until_token(buffer, token, delim) == -13;
}

TEST_GENERATE(
	//tokenize tests
	test1, test2, test3, test4,  test5,  test6,
	test7, test8, test9, test10, test11, test12
);
