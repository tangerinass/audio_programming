#ifndef READ_FIFO_
#define READ_FIFO_
#include <fcntl.h>
#include <unistd.h>

int read_fifo(char *path, char *buf, int size);

#endif
