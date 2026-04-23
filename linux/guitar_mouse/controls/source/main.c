#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../../common/common.h"

#include <semaphore.h>

#define WAIT_TIME_N 100000

sem_t msg_rec;
sem_t msg_applied;

void* read_pipe(void* args){
	int sent = 0, fd, ret;
	char buf[BUFSIZ];
	int* controls = (int*)args;
	
	while (1){
		if (sent){
			sent = 0;
			sem_wait(&msg_applied);
		}
		
		if ( (fd = open(pathname, O_RDONLY)) < 0){
			printf("Cant connect to fifo\n");
			continue;
		}

		ret = read(fd, buf, MSG_SIZE);
		
		close(fd);

		if (ret < 0){
			perror("Failed trying to read fifo");
			exit(-1);
		}
		else if (ret > 0){

			printf("Message recieved!\n");
				
			ret = apply_msg_controls(buf, controls);
			
			sem_post(&msg_rec);
			 
			sent = 1;

			if (ret < 0){
				printf("Sender disconected");
				sent = 0;
			}
		}
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
	int controls = NONE;
	struct libevdev *device;
	struct libevdev_uinput *uidevice;

	pthread_t pipe_t;
	
	sem_init(&msg_rec, 0, 0);
	sem_init(&msg_applied, 0, 1);

	uidevice = setup_devices(device);

	if (ret != 0){
		exit(-1);
	}
	
	pthread_create(&pipe_t, NULL, read_pipe, (void*) &controls);

	while (1){
		sem_wait(&msg_rec);
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
				usleep(WAIT_TIME_N);	
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_LEFT, 0);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case RIGHT_B:
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_RIGHT, 1);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				usleep(WAIT_TIME_N);	
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_RIGHT, 0);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
			case MIDDLE_B:
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_MIDDLE, 1);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				usleep(WAIT_TIME_N);	
				libevdev_uinput_write_event(uidevice, EV_KEY, BTN_RIGHT, 0);
				libevdev_uinput_write_event(uidevice, EV_SYN, SYN_REPORT, 0);
				break;
		}
		sem_post(&msg_applied);
	}

	pthread_join(pipe_t, NULL);

	libevdev_uinput_destroy(uidevice);
	
	return 0;
}
