#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../common/common.h"

#include <signal.h>
// https://docs.kernel.org/input/event-codes.html

static volatile int run = 1;

void sig_handler(int x){
	run = 0;
}

/* TODO: fazr mensganes de erro para os pipes */
void* read_pipe(void* args){
	int fd, ret;
	char buf[BUFSIZ];
	int* controls = (int*)args;

	ret = mkfifo(pathname, 0666);
	chown(pathname, 1000, 1000);
	if (ret != 0){
		perror("Failed trying to create fifo");
		return NULL;
	}
	
	while (run){
		
		if ( (fd = open(pathname, O_RDONLY)) < 0){
			perror("Failed trying to open fifo");
			return NULL;
		}

		ret = read(fd, buf, MSG_SIZE);
		
		close(fd);

		if (ret < 0){
			perror("Failed trying to read fifo");
			return NULL;
		}
		else if (ret > 0){
			printf("Message recieved!\n");
			apply_msg_controls(buf, controls);
		} else {
			printf("Message not receieved\n");
			controls[0] = -1;
		}
		usleep(10000);
	}
	return NULL;
}

struct libevdev_uinput* setup_devices(struct libevdev *device){
	int ret;
	struct libevdev_uinput *uidevice;

	// defenir um device
	// o gajo n funciona se n tiver as capablidades de um rato normal (tem de consguir clicr e mover)
	device = libevdev_new();
	libevdev_set_name(device, "New Device");

	// defenir oqq o device pode fazr (tipo de evento)
	// movimento do rato
	libevdev_enable_event_type(device, EV_REL);
	libevdev_enable_event_code(device, EV_REL, REL_X, NULL);
	libevdev_enable_event_code(device, EV_REL, REL_Y, NULL);

	// pressionar botao do rato
	libevdev_enable_event_type(device, EV_KEY);
	libevdev_enable_event_code(device, EV_KEY, BTN_LEFT, NULL);
	libevdev_enable_event_code(device, EV_KEY, BTN_RIGHT, NULL);
	libevdev_enable_event_code(device, EV_KEY, BTN_MIDDLE, NULL);

	libevdev_enable_event_type(device, EV_SYN);
	libevdev_enable_event_code(device, EV_SYN, SYN_REPORT, NULL);

	ret = libevdev_uinput_create_from_device(device, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidevice);
	if (ret != 0){
		perror("Failed trying to create virtual uidevice");
		return NULL;
	}

	return uidevice;

}

void cleanup_devices(struct libevdev *device, struct libevdev_uinput *uidevice){
	libevdev_free(device);
	libevdev_uinput_destroy(uidevice);
	return;
}

int main(){
	int ret;
	int msg[2] = {0,0};
	struct libevdev *device;
	struct libevdev_uinput *uidevice;

	pthread_t pipe_t;

	uidevice = setup_devices(device);

	// TODO: dar handle a caso de erro aqui 
	if (ret != 0){
		exit(-1);
	}

	pthread_create(&pipe_t, NULL, read_pipe, (void*) &msg);

	signal(SIGINT, sig_handler);

	sleep(1);

	printf("Script started\n");
	while (run){
		int dir = msg[0];
		int dist = msg[1];
		printf("MSG:[%d,%d]",msg[0], msg[1]);
		if (dir != -1){
			libevdev_uinput_write_event(uidevice, EV_REL, dir, dist);
			libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
			usleep(100000);
		}
	}


	pthread_join(pipe_t, NULL);
	cleanup_devices(device, uidevice);
	printf("Script ended\n");
	return 0;
}
