#include "common.h"

const char* pathname = "/tmp/fifo";

// idk so vou deixar isto assim por enquanto
int apply_msg_controls(char *msg, int *controls){
	controls[0] = ((int)msg[0])%2;
	controls[1] = (int)msg[0];
	return 0;
}
