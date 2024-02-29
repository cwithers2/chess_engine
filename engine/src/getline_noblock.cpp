//C++ version
#ifdef __cplusplus
#include <cstdio>
#include <iostream>
int getline_noblock_init(){
	std::cin.sync_with_stdio(false);
	return 0;
}

char* getline_noblock(char* buffer, int len){
	auto&& count = std::cin.rdbuf()->in_avail();
	if(count <= 0)
		return NULL;
	if(!fgets(buffer, len, stdin))
		return NULL;
	return buffer;
}
// C Linux
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
int getline_noblock_init(){
	int r;
	r = fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
	return r != -1;
}

char* getline_noblock(char* buffer, int len){
	if( (read(0, buffer, len)) > 0 )
		return buffer;
	return NULL;
}
// C Windows
#elif _WIN32
#include <Windows.h>
int getline_noblock_init(){
	return 0;
}

char* getline_noblock(char* buffer, int len){
	DWORD r;
	GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &r);
	if(r <= 0)
		return NULL;
	if(!fgets(buffer, len, stdin))
		return NULL;
	return buffer;
}
// C Unknown
#else
#warning OS not supported.
#warning Falling back to blocking implementation.
#include <stdio.h>
int getline_noblock_init(){
	return 0;
}

char* getline_noblock(char* buffer, int len){
	if(!fgets(buffer, len, stdin))
		return NULL;
	return buffer;
}
#endif

