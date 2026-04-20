#ifndef MUSIC_STRUCTS_
#define MUSIC_STRUCTS_

// Definições de limites
#define CHORD_SIZE 5
#define MAX_CHORDS 10
#define NOTE_SIZE 3
#define KEYBOARD_SIZE 13
# define M_PI		3.14159265358979323846	/* pi */


// Defenições de Notas e Accordes
typedef enum Note { 
    Inv, // Valor base
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
} Note;

const static float NOTE_FREQS[] = {0, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

typedef struct Chord {
    Note    note_array[CHORD_SIZE];
    int size;
} Chord;

// Defenições de funções das notas e accordes

char* get_note_string(Note note);

Note get_note_enum(char *in);

int print_chord(Chord *chord);

Note get_note_enum_keyboard(char in, char* keyboard);

float get_freq(Note note);

#endif
