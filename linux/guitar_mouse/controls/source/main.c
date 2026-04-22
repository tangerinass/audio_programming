#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include "../../common/common.h"

#include <signal.h>
// https://docs.kernel.org/input/event-codes.html

volatile sig_atomic_t running = 1;

void sig_handler(int x){
	running = 0;
}

/* TODO: fazr mensganes de erro para os pipes */
void* read_pipe(void* args){
	int fd, ret;
	char buf[BUFSIZ];
	int* controls = (int*)args;

	ret = mkfifo(pathname, 0666);
	// TODO: encontrar modo de encontrar o user e grp de um user (acho q default é isto mas n é mt bom assim)
	chown(pathname, 1000, 1000);
	if (ret != 0){
		perror("Failed trying to create fifo");
		return NULL;
	}
	
	while (running){
		
		if ( (fd = open(pathname, O_RDONLY| O_NONBLOCK)) < 0){
			
			if (errno != EAGAIN){
				perror("Failed trying to open fifo");
				return NULL;
			}
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

int main(){
	int ret;
	int controls = {NONE};
	struct libevdev *device;
	struct libevdev_uinput *uidevice;

	pthread_t pipe_t;

	signal(SIGINT, sig_handler);
	
	uidevice = setup_devices(device);

	// TODO: dar handle a caso de erro aqui 
	if (ret != 0){
		exit(-1);
	}

	pthread_create(&pipe_t, NULL, read_pipe, (void*) &controls);

	sleep(1);

	printf("Script started (Press Crtl-C to interrupt)\n");
	while (running){
		switch (controls){
			case NONE:
				break;
			case LEFT:
				libevdev_uinput_write_event(uidevice, EV_REL, REL_X, DIST);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case RIGHT:
				libevdev_uinput_write_event(uidevice, EV_REL, REL_X, (-1) * DIST);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case UP:
				libevdev_uinput_write_event(uidevice, EV_REL, REL_Y, DIST);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case DOWN:
				libevdev_uinput_write_event(uidevice, EV_REL, REL_Y, (-1) * DIST);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case LEFT_B:
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_LEFT, 1);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				usleep(100000);	
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_LEFT, 0);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case RIGHT_B:
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_RIGHT, 1);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				usleep(100000);	
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_RIGHT, 0);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case MIDDLE_B:
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_MIDDLE, 1);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				usleep(100000);	
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_RIGHT, 0);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
		}
		usleep(100000);
	}

	printf("Script ended\n");
	
	pthread_join(pipe_t, NULL);
	
	unlink(pathname);
	
	libevdev_uinput_destroy(uidevice);
	
	return 0;
}
