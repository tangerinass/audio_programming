#ifndef PARSING_FUNCS_
#define PARSING_FUNCS_

#include "music.h"

// returns 1 if theres an error readin the buffer
int read_chord(char* in, Chord *chord);

// return 1 if theres an errror opening or closing the file
int write_array_to_file(char* name, FILE* fptr, Chord** arr, int size);

// return the number of chords read, if theres an errror opening or closing the file returns -1
int read_array_from_file(char* name, FILE* fptr, Chord** arr);


#endif