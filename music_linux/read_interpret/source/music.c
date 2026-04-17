#include "music.h"
#include <math.h>
#include <stdlib.h>
const char* NOTES[NOTES_SIZE] = {"A", "Bb", "B", "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab"};

const char* get_note_string(float freq){
	int offset = calc_offset(freq);
	return NOTES[abs(offset%NOTES_SIZE)];
}

int calc_offset(float freq){
	return NOTES_SIZE*log2(freq/REFERENCE_FREQ);
}
