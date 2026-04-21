#include "read_fifo.h"
// returns 0 in case of sucess, -1 in case of error
int read_fifo(char *path, char *buff, int size){
	int ret, fd;

	fd = open(path, O_RDONLY);
	
	if (fd < 0){
		return -1;
	}

	ret = read(fd, buff, size);
	
	if (ret < 0){
		return -1;
	}
	
	close(fd);
	return 0;
}
