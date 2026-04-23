#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <signal.h>

#include "constants.h"
#include "read_fifo.h"
#include "../../common/common.h"

volatile sig_atomic_t running = 1;

struct winsize window;

void handle_interr(int x){
    running = 0;
}

void handle_resize(int x){
    ioctl(0 , TIOCGWINSZ, &window);
}

void draw_guitar(char** buff, int offset){
    
}

int main(){
    char** terminal_buff;
    
    signal(SIGINT, handle_interr);
    signal(SIGWINCH, handle_resize);

    ioctl(0 , TIOCGWINSZ, &window);
    
    terminal_buff = malloc(sizeof(char*)*TERMINAL_SIZE);

    for (int i = 0; i < window.ws_row; i++){
        terminal_buff[i] = malloc(sizeof(char)*TERMINAL_SIZE);
    }

    while (running){

        // despejar o buffer no terminal
        for (int i = 0; i < window.ws_row; i++){
            printf("\033[9m%s\033[0m\n",terminal_buff[i]);
        }

    }

    return 0;
}
