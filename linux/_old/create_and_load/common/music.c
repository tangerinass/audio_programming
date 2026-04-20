#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "music.h"

// converte char de uma lista especifica para notas
Note get_note_enum_keyboard(char in, char* keyboard){
    if (in == keyboard[0]) return C;
    if (in == keyboard[1]) return Db;
    if (in == keyboard[2]) return D;
    if (in == keyboard[3]) return Eb;
    if (in == keyboard[4]) return E;
    if (in == keyboard[5]) return F;
    if (in == keyboard[6]) return Gb;
    if (in == keyboard[7]) return G;
    if (in == keyboard[8]) return Ab;
    if (in == keyboard[9]) return A;
    if (in == keyboard[10]) return Bb;
    if (in == keyboard[11]) return B;
    return Inv;
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
        default:
            return "";
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
    int i;
    for (i = 0; i < chord->size;i++){
        printf("%s ",get_note_string(chord->note_array[i]));
    }
    printf("\n");
    return 0;
}

float get_freq(Note note){
    return NOTE_FREQS[note];
};
