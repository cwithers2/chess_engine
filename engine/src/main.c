#include <uci.h>
#include <threads.h>
#include <stdio.h>
int main(int argc, char** argv){
	uci_init();
	thrd_sleep(&(struct timespec){.tv_sec=10}, NULL);
		if(uci_poll(&argc, &argv))
			for(int i = 0; i < argc; ++i)
				printf("%i: %s\n", i, argv[i]);
	uci_destroy();
}
