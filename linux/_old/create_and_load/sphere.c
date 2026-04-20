#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <string.h>

#include "common/music.h"
#include "common/parse.h"
#include "common/msg.h"

// mexer nestes valores para depois mudar a vizualização
// mudar a formula q tenoh para normalizar, ou talvez tmb mudar o simoblo q uso dependendo do tmanho vertical

#define R 1
#define SCREEN_DIST 10
#define INTERVAL 100

// define a instnsidade / desidade do char

int main(int argc, char* argv[]){
    int arr_size, i, j, arr_ptr;
    char c, *console_buffer_c, name[NAME_SIZE + EXTENSION_SIZE + FOLDER_SIZE];
    struct winsize terminal_size;
    FILE* fptr;
    float *console_buffer_f;

    if (argc < 2){
        printf("Escreve o nome de um ficheiro .sdat para correr com o programa\n./<nome do executavel> <nome do ficheiro sdat>\n");
        return 1;
    }
    // guardar o nome do fichieor do argv

    strcpy(name, FOLDER_STR);

    strcat(name, argv[1]);
    
    strcat(name, EXTENSION_STR);
    Chord** chord_arr = malloc(sizeof(Chord*)*MAX_CHORDS);

    if ((arr_size = read_array_from_file(name, fptr, chord_arr)) < 0){
        printf("%s\n", ERROPENFILE);
        free(chord_arr);
        return 1;
    }
    // ir busca as env variables do terminal_size (tamanho em pixeis e col/linhas)
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size);

    // criar uma tabela que vai simular a consola
    console_buffer_c = malloc(sizeof(char)*terminal_size.ws_row*terminal_size.ws_col);

    console_buffer_f = malloc(sizeof(float)*terminal_size.ws_row*terminal_size.ws_col);
    
    float A = 0, B = 0;
    float theta, phi;

    for(;;) {
	j++;
	if (j == INTERVAL){
	     j = 0;
	     arr_ptr = (arr_ptr + 1) % (arr_size);
	}	
        memset(console_buffer_c,32,terminal_size.ws_row*terminal_size.ws_col);
        memset(console_buffer_f,0,terminal_size.ws_row*terminal_size.ws_col*4);
	for(theta=0; theta < 6.28; theta += 0.07) {
             for(i = 0; i < chord_arr[arr_ptr]->size ; i++) {
	     phi = (6.28 / 12) * chord_arr[arr_ptr]->note_array[i]; 
	     float sin_theta = sin(theta);
	     float sin_phi = sin(phi);
	     float sin_A = sin(A);
	     float sin_B = sin(B);
	    
             float cos_theta = cos(theta);
	     float cos_phi = cos(phi);
 	     float cos_A = cos(A);
	     float cos_B = cos(B);

	     // calcular as posiçoes dos pontos da esfera
	     float x = R * cos_theta * cos_phi * cos_B - sin_B *
		    (R * sin_theta * cos_A - R * cos_theta * sin_phi * sin_A);
	    
	     float y = R * cos_theta * cos_phi * sin_B + cos_B *
		    (R * sin_theta * cos_A - R * cos_theta * sin_phi * sin_A);
	     
	     float z = SCREEN_DIST + R * sin_theta * sin_A + R * cos_theta * sin_phi * cos_A;
	     
	     float ooz = 1.0/z;
	    // calcular as projeçoes
	    // deifine a escala (posiçao no ecra, aparencia da proj)
	     float scaling = terminal_size.ws_row * SCREEN_DIST * (3.0/8.0) * (1.0/R);

	     int xp = (terminal_size.ws_col/2 + x*scaling*2*ooz);

	     int yp = (terminal_size.ws_row/2 - y*scaling*ooz);
	     
	 
	     // calcular o nivel de luz
	     float L = cos_phi*cos_theta*sin_B
		     - cos_A * cos_theta * sin_phi
		     - sin_A * sin_theta + cos_B
		     *  (cos_A*sin_theta - cos_theta*sin_A*sin_phi);
	     
	     L *= 8;

	     int arr_ptr = xp + terminal_size.ws_col * yp;
	     if(arr_ptr < terminal_size.ws_row * terminal_size.ws_col
			     && xp > 0 && yp > 0
			     && xp < terminal_size.ws_col
			     && yp < terminal_size.ws_row
			     && ooz > console_buffer_f[arr_ptr]){
		     console_buffer_f[arr_ptr] = ooz;
		     console_buffer_c[arr_ptr] = ".,-~:;=!*#$@"[L > 0 ? (int) L : 0];
                 }
             }
         }
	 printf("\x1b[H");
       	 for(i = 0; i < terminal_size.ws_col* terminal_size.ws_row + 1; i++) {
	     printf("\033[9%dm%c\033[0m", i % 7,terminal_size.ws_col ? console_buffer_c[i] : '\n');
	     //putchar(terminal_size.ws_col ? console_buffer_c[i] : 10);
	     A += 0.00004/2;
             B += 0.00002/2;
         }
	 putchar(10);
         usleep(30000);
    }
    for(i = 0; i < arr_size; i++){
        free(chord_arr[i]);
    }
    free(console_buffer_c);
    
    free(console_buffer_f);
    
    return 0;
}
