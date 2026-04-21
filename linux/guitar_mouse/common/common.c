#include "common.h"

enum CONTROLS controls;

const char* pathname = "/tmp/fifo";

// idk so vou deixar isto assim por enquanto
int apply_msg_controls(char *msg, int *controls){
	controls[0] = (int)msg[0] % (MAX_CONTROLS-1); 
	return 0;
}
