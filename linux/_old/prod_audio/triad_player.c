#define MINIAUDIO_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "miniaudio.h"
#include <math.h>
#include <time.h>

// Definições de limites
#define MAXCHORDSIZE 3
#define MAXCHORDS 10
#define CHORDSIZE 20
#define NOTESIZE 20
#define CHORDTIME 50

// Mensagens de erro
#define ERRREADCHORD    "erro ao ler o accorde, verifique o formato do accorde inserido"
#define ERRREADPOS      "erro ao ler a posição, verifique a posição do accorde que quer remover"
#define LIMITCHORDS     "limite maximo de accordes atingido"
#define ERRSETUPAUDIO   "erro ao tentar dar setup ao audio"
#define ERRPLAYAUDIO    "erro ao tentar tocar o audio"
#define ERRSTOPAUDIO    "erro ao tentar parar o audio"
#define ERRNOCHORD      "não tem accordes guardados para tocar"

#define SAMPLE_RATE 48000 // taxa de amostragem de áudio

// Enumeração das notas musicais
typedef enum Note { 
    C, 
    Db, 
    D, 
    Eb, 
    E, 
    F, 
    Gb, 
    G,
    Ab,
    A, 
    Bb,
    B,
    Inv = -1, // Nota inválida
} Note;

// Tabela de frequências das notas
float NOTE_FREQS[] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

// Estrutura que representa um acorde (máximo 3 notas)
typedef struct Chord {
    Note  note_array[MAXCHORDSIZE];
} Chord;

// Estrutura principal que guarda as informações do programa
typedef struct Info {
    ma_device_config audio_config;
    Chord** chord_array;
    int array_size;
    int array_ptr;
    int chord_time;
    double t;
} Info;

// Imprime as instruções de uso
int write_instructions(){
    printf("ADD --------\n+ <note>-<note>-<note>\n");
    printf("REMOVE --------\n- <nºposition>\n");
    printf("LIST --------\nl\n");
    printf("QUIT --------\nq\n");
    return 0;
}

// Devolve a frequência da nota
float get_freq(Note note){
    return NOTE_FREQS[note];
}

// Converte enum para string da nota
char* get_note_string(Note note){
    switch (note)
    {
        case A: 
            return "A"; 
        case Ab: 
            return "Ab";
        case B: 
            return "B"; 
        case Bb: 
            return "Bb"; 
        case C:
            return "C";
        case D: 
            return "D";
        case Db: 
            return "Db"; 
        case E: 
            return "E"; 
        case Eb: 
            return "Eb"; 
        case F: 
            return "F"; 
        case G: 
            return "G"; 
        case Gb: 
            return "Gb"; 
    }
}

// Converte string para enum da nota
Note get_note_enum(char *in){
    if (strcmp(in, "A") == 0) return A;
    if (strcmp(in, "Ab") == 0) return Ab;
    if (strcmp(in, "B") == 0) return B;
    if (strcmp(in, "Bb") == 0) return Bb;
    if (strcmp(in, "C") == 0) return C;
    if (strcmp(in, "D") == 0) return D;
    if (strcmp(in, "Db") == 0) return Db;
    if (strcmp(in, "E") == 0) return E;
    if (strcmp(in, "Eb") == 0) return Eb;
    if (strcmp(in, "F") == 0) return F;
    if (strcmp(in, "G") == 0) return G;
    if (strcmp(in, "Gb") == 0) return Gb;
    return Inv;
}

// Imprime um acorde
int print_chord(Chord *chord){
    printf("%s-%s-%s\n",
        get_note_string(chord->note_array[0]),
        get_note_string(chord->note_array[1]),
        get_note_string(chord->note_array[2]));
    return 0;
}

// Adiciona um acorde à lista
int add_chord(char* in, Info *info){
    int i, ptr_note, ptr_arr;
    char chord_str[CHORDSIZE], note_str[NOTESIZE];
    Chord* new;
    
    if (info->array_size == MAXCHORDS){
        printf("%s", ERRREADCHORD);
        return 1;
    }
    if (sscanf(in,"%*c %s",chord_str) < 1){
        printf("%s:%s",chord_str, ERRREADCHORD);
        return 1;
    }
    
    new = malloc(sizeof(Chord));

    for (i = 0, ptr_note = 0, ptr_arr = 0; i < strlen(chord_str); i++){
        if (chord_str[i] == '-'){
            new->note_array[ptr_arr++] = get_note_enum(note_str);
            ptr_note = 0;
            continue;
        }
        note_str[ptr_note++] = chord_str[i];
        note_str[ptr_note] = '\0';
    }
    new->note_array[ptr_arr++] = get_note_enum(note_str);
    info->chord_array[info->array_size] = new;
    info->array_size++;

    printf("Chord added!\n");
    return 0;
}

// Lista todos os acordes guardados
int list_progression(char* in, Info *info){
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

// Função de callback de áudio chamada automaticamente pelo MiniAudio
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    Info* data = (Info*)pDevice->pUserData;
    float* out = (float*)pOutput;

    for (ma_uint32 i = 0; i < frameCount; i++) {
        // Soma os 3 tons das notas do acorde
        double sample1 = sin(get_freq(data->chord_array[data->array_ptr]->note_array[0]) * data->t * 2 * M_PI);
        double sample2 = sin(get_freq(data->chord_array[data->array_ptr]->note_array[1]) * data->t * 2 * M_PI);
        double sample3 = sin(get_freq(data->chord_array[data->array_ptr]->note_array[2]) * data->t * 2 * M_PI);
        float mixed = (float)((sample1 + sample2 + sample3) /3);

        data->t += 1.0 / SAMPLE_RATE;

        *out++ = mixed; // canal esquerdo
        *out++ = mixed; // canal direito
    }

    // Passa para o próximo acorde depois de algum tempo
    if (data->chord_time++ > CHORDTIME){
        data->chord_time = 0;
        data->array_ptr++;
    }

    // Reinicia o ciclo se acabou os acordes
    if (data->chord_array[data->array_ptr] == NULL) data->array_ptr = 0;

    (void)pInput; // não usado
}

// Começa a tocar os acordes
int play(Info *info){
    ma_device device;
    if (info->array_size < 1){
        printf("%s\n", ERRNOCHORD);
        return -1;
    }

    info->audio_config.pUserData = info;

    if (ma_device_init(NULL, &info->audio_config, &device) != MA_SUCCESS) {
        printf("%s\n", ERRSETUPAUDIO);
        return -1;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("%s\n",ERRPLAYAUDIO);
        ma_device_uninit(&device);
        return -1;
    }

    printf("carregue enter para parar\n");
    getchar(); // espera enter

    ma_device_uninit(&device);
    printf("audio desligado\n");
    return 0;
}

// Liberta memória e termina o programa
void end(Info *info){
    for (int i = 0; i < info->array_size; i++){
        free(info->chord_array[i]);
    }
    free(info->chord_array);
    exit(0);
}

// Função principal
int main(int argc, char* argv[]) {
    char buf[BUFSIZ];
    Info program_info;

    // Configurações de áudio
    program_info.audio_config = ma_device_config_init(ma_device_type_playback);
    program_info.audio_config.sampleRate = SAMPLE_RATE;
    program_info.audio_config.playback.format = ma_format_f32;
    program_info.audio_config.playback.channels = 2;
    program_info.audio_config.dataCallback = data_callback;

    // Inicializa array de acordes
    program_info.chord_array = calloc(MAXCHORDS,sizeof(Chord*));
    program_info.array_size = 0;

    // Mostra instruções
    write_instructions();

    // Loop principal que lê comandos
    while (fgets(buf, BUFSIZ, stdin))
    {
        switch (buf[0])
        {
        case '+': add_chord(buf,&program_info);break;
        case '-': remove_chord(buf, &program_info);break;
        case 'l': list_progression(buf,&program_info);break;
        case 'h': write_instructions(); break;
        case 'p': play(&program_info);break;
        case 'q': end(&program_info);
        default: break;
        }
    }

    return 0;
}
