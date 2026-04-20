#include "common.h"

const char* pathname = "../tmp/fifo";

// idk so vou deixar isto assim por enquanto
int apply_msg_controls(int msg, int *controls){
	controls[0] = msg%2;
	controls[1] = msg%12;
	return 0;
}
