#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common/music.h"
#include "common/parse.h"
#include "common/msg.h"

// Estrutura principal que guarda as informações do programa
typedef struct Info {
    Chord** chord_array;
    int     array_size;
    int     is_connected;
} Info;
struct args_thread {
        char *keyboard;
        Chord *chord;
};
// limpa o terminal
void clear_terminal(){

    printf("\e[1;1H\e[2J");

    return;
}

// Imprime as instruções de uso
int write_instructions(){
    clear_terminal();
    printf("Adding chords:\n> a <note> <note> <note>\n");
    printf("Removing chords:\n> r <nºposition>\n");
    printf("Listing all the chords:\n> l\n");
    printf("Get info:\n> i\n");
    printf("Recording in piano mode:\n> p\n");
    printf("Save to a file:\n> s <filename>\n");
    printf("Loading from a file:\n> g <filename>\n");
    printf("Get help:\n> h\n");
    printf("Exit:\n> q\n");
    return 0;
}

// Adiciona um acorde à lista
// fazer isto verificar erros
int add_chord(char* in, Info *info){
    Chord* new_chord;

    if (info->array_size == MAX_CHORDS){
        printf("%s", ERRLIMITCHORDS);
        return 1;
    }
    
    new_chord = calloc(1,sizeof(Chord));
    
    if (read_chord(in, new_chord)){
        printf("%s\n",ERRREADCHORD);
        free(new_chord);
        return 1;
    }

    info->chord_array[info->array_size] = new_chord;
    info->array_size++;
    
    printf("Chord Added:\n");
    print_chord(new_chord);
    return 0;
}

// Lista todos os acordes guardados
int list_progression(Info *info){
    int i;
    for (i = 0; i < info->array_size; i++){
        printf("%d. ",(i+1));
        print_chord(info->chord_array[i]);
    }
    return 0;
}

// Remove um acorde pela posição
int remove_chord(char* in, Info *info){
    int i, pos;

    if (sscanf(in,"%*c %d", &pos) < 1){
        printf("%s\n", ERRREADPOS);
        return 1;
    }
    if (pos > info->array_size){
        printf("%s\n", ERRREADPOS);
        return 1;
    }

    pos--;
    free(info->chord_array[pos]);
    for (i = pos; i < info->array_size - 1; i++){
        info->chord_array[i] = info->chord_array[i+1];
    }
    info->chord_array[info->array_size] = NULL;
    info->array_size--;

    printf("Chord removed!\n");

    return 0;
}

// Liberta memória e termina o programa
void end(Info *info){
    for (int i = 0; i < info->array_size; i++){
        if (info->chord_array[i] == NULL){
            break;
        }
        free(info->chord_array[i]);
    }
    free(info->chord_array);
    exit(0);
}

// Guarda para um ficheiro com extensão ".sdat" a array de accordes 
int save_to_file(char* in, Info* info){
    int i, j;
    FILE* fptr;
    char tmp[NAME_SIZE], name[NAME_SIZE + EXTENSION_SIZE + FOLDER_SIZE];

    // verifica se existe ou n uma pasta dedicada, senao existir ent cria uma na dir atual

    struct stat st = {0};

    if (stat(FOLDER_STR, &st) == -1) {
        mkdir(FOLDER_STR, 0700);
    }

    strcpy(name, FOLDER_STR);

    if (sscanf(in,"%*c %s", tmp) < 1){
        printf("%s\n", ERRREADNAME);
        return 1;
    }

    strcat(name, tmp);

    strcat(name, EXTENSION_STR);

    if (write_array_to_file(name, fptr, info->chord_array, info->array_size)){
        printf("%s\n", ERRCREATEFILE);
        return 1;
    }

    printf("Done\n");
}

// Guarda para array de accordes os dados de um ficheiro com extensão ".sdat"
int load_from_file(char* in, Info* info){
    char tmp[NAME_SIZE], name[NAME_SIZE + EXTENSION_SIZE + FOLDER_SIZE], buf[BUFSIZ];
    FILE* fptr;

    strcpy(name, FOLDER_STR);

    if (sscanf(in,"%*c %s", tmp) < 1){
        printf("%s\n", ERRREADNAME);
        return 1;
    }
    
    strcat(name, tmp);

    strcat(name, EXTENSION_STR);

    if ((info->array_size = read_array_from_file(name, fptr, info->chord_array)) < 0){
        printf("%s\n", ERROPENFILE);
        return 1;
    }

    printf("Done\n");
    return 0;
}

int get_info(Info* info){
    printf("Tamanho de accordes maximo:%d\n", CHORD_SIZE);
    printf("Numero maximo de accordes possiveis:%d\n", MAX_CHORDS);
    printf("Numero de accordes guardados:%d\n",info->array_size);
    return 0;
}

int setup_keyboard_piano(char* arr){
    int i;
    char input;
    system ("/bin/stty raw");
    for (i = 0; i < (int)B; i++){
        printf("Type a key for %s:\n\r", get_note_string(i + 1));
        input = getchar();
        clear_terminal();
        arr[i] = input;
    }
    system ("/bin/stty cooked");
    return 0;
}

int message_terminal_record(Chord *chord){
    clear_terminal();
    printf("carregue '.' para parar carregue '-' para apagar o accorde e '+' para guardar o accorde\n\r");
    printf("Accorde atual: ");
    print_chord(chord);
    printf("\r");
}
int record_notes(void* args){
    int i;
    char *keyboard, c;
    Note note;
	Chord *chord;
	
	struct arg_thread *a = args;

    system ("/bin/stty raw");
    
	while(c=getchar()) {

        switch (c)
        {
        case '.':
            goto end_recording;
            return 0;
        case '-':
            for (i=0; i < chord->size; i++){
                chord->note_array[i] = '\0';
            }  
            break;
        default:
            if (chord->size == CHORD_SIZE) {
                for (i=0; i < CHORD_SIZE; i++){
                    chord->note_array[i] = '\0';
                }
                chord->size = 0;
            }
            note = get_note_enum_keyboard(c, keyboard);
            if (note != Inv){
                chord->note_array[chord->size++] = note;
            }
            break;
        }
    }

    end_recording:
    
    system ("/bin/stty cooked");

    return 0;
}

int piano_visualizer(Info *info){
    char *keyboard;
    Chord *chord;
	pthread_t thread;
	struct args_thread args;

    chord = calloc(1,sizeof(Chord));

    keyboard = malloc(KEYBOARD_SIZE*sizeof(char));

	args.keyboard = keyboard;
	args.chord = chord;

    clear_terminal();
	
    setup_keyboard_piano(keyboard);
	
	// fazer um thread differnte q le do chord
	pthread_create(&thread, NULL, record_notes, (void *)(&args));
	
	pthread_join(thread, NULL);
	
	// fazr aqui o loop a espera q o phtread faça join
    return 0;
}
int main(int argc, char* argv[]) {
    char buf[BUFSIZ];
    Info program_info;

    program_info.chord_array = malloc(MAX_CHORDS*sizeof(Chord*));
    program_info.array_size = 0;
    program_info.is_connected = 0;
    // Mostra instruções
    write_instructions();

    // Loop principal que lê comandos
    // fgets apanha o enter ent a string acabar em "....\n\0" meio estranho
    while (fgets(buf, BUFSIZ, stdin))
    {
        switch (buf[0])
        {
        case 'a': add_chord(buf,&program_info);break;
        case 'r': remove_chord(buf, &program_info);break;
        case 'l': list_progression(&program_info);break;
        case 'h': write_instructions(); break;
        case 'i': get_info(&program_info);break;
        case 'q': end(&program_info);
        case 'p': piano_visualizer(&program_info);break;
        case 's': save_to_file(buf, &program_info);break;
        case 'g': load_from_file(buf, &program_info);break;
        default: break;
        }
    }

    return 0;
}
