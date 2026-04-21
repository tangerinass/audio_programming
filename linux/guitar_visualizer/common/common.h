#ifndef COMMON_
#define COMMON_

#define MSG_SIZE 20
#define DIST 50

enum CONTROLS{
	NONE = -1,
	LEFT,
	UP,
	RIGHT,
	DOWN,
	LEFT_B,
	RIGHT_B,
	MIDDLE_B
};

extern enum CONTROLS controls;
#define MAX_CONTROLS 8

extern const char* pathname;

int apply_msg_controls(char *msg, int *controls);

#endif
