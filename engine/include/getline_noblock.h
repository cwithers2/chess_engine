#ifndef GETLINE_NOBLOCK
#define GETLINE_NOBLOCK
#ifdef __cplusplus
extern "C"
#endif
int getline_noblock_init();
#ifdef __cplusplus
extern "C"
#endif
char* getline_noblock(char* buffer, int len);

#endif
