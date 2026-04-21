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

#define DEVIATION 0.7
#define WAIT_TIME 30000
#define MAX_PHASE 300
#define NORMALIZE_FREQ 80
#define MAX_ROWS 56

#define RED_COLOR "\033[91m"

// define a instnsidade / desidade do char
const char* CHAR_DENISTY = "$@B%%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,^`'.";

int generate_sine_wave(Chord* chord_info, double phase, struct winsize size, char** buf){

    int i, j, k, index;

    double value;

    index = strlen(CHAR_DENISTY) - (size.ws_row * strlen(CHAR_DENISTY))/MAX_ROWS;


    for (i = 0; i < size.ws_row; i++){

        for(j = 0; j < size.ws_col; j++){

            // merda q acabou por funcionar

            value = 0;

            // fazer a soma de todas as ondas e normalizar para caber no termial

            for (k = 0; k < chord_info->size; k++){

                value += sin(2 * M_PI * (get_freq(chord_info->note_array[k]) / NORMALIZE_FREQ) * ((phase + j) / size.ws_col) ) * (size.ws_row / ((chord_info->size+1)*2));

            }

            // por a onda no meio da consola
            value += size.ws_row/2;

            // se o "ponto" estiver perto da onda ent "pintar" o "ponto" na consola 
            if (fabs(i - value) < DEVIATION){

                buf[i][j] = CHAR_DENISTY[index];

                continue;   

            }

            // else por espaço

            buf[i][j] = ' ';

        }

    }
    
    return 0;
}

// banger code fr

int main(int argc, char* argv[]){

    int i, j, phase, arr_size, ptr = 0;
    char c, **console_buffer, name[NAME_SIZE + EXTENSION_SIZE + FOLDER_SIZE];
    struct winsize terminal_size;
    FILE* fptr;

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
    console_buffer = malloc(sizeof(char*)*terminal_size.ws_row);

    for (i = 0; i < terminal_size.ws_row; i++){
        console_buffer[i] = malloc(sizeof(char)*terminal_size.ws_col);
    }

    phase = 0;
    
    while (phase < MAX_PHASE){
        phase += 1;

        if (phase % (MAX_PHASE / (arr_size)) == 0 && ptr < arr_size - 1){
            ptr++;
        }

        // limpar o ecra
        printf("\e[2J");

        // preencher o buffer
        generate_sine_wave(chord_arr[ptr] ,phase, terminal_size, console_buffer);

        // despejar o buffer no terminal
        for (i = 0, j = 0; i < terminal_size.ws_row; i++, j++, j = j%7){
            printf("\033[9%dm%s\033[0m\n",j,console_buffer[i]);
        }
        // esperar para parecer mais "limpo" 
        usleep(WAIT_TIME);  

    }
    for(i = 0; i < arr_size; i++){
        free(chord_arr[i]);
    }
    
    for (i = 0; i < terminal_size.ws_row; i++){
        free(console_buffer[i]);
    }

    free(console_buffer);
    
    return 0;
}
