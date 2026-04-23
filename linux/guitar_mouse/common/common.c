#include "common.h"
#include <stdlib.h>
enum CONTROLS controls;

const char* pathname = "/tmp/fifo";

// idk so vou deixar isto assim por enquanto
int apply_msg_controls(char *msg, int *controls){
	int x = atoi(msg);
	
	if (x == -1) return -1;

	controls[0] = x % (MAX_CONTROLS-1); 
	
	return 0;
}
