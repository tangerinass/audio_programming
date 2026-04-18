#include "music.h"
#include <math.h>
#include <stdlib.h>
const char* NOTES[NOTES_SIZE] = {"A", "Bb", "B", "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab"};

const char* get_note_string(float freq){
	int offset = calc_offset(freq);
	int i = (offset % NOTES_SIZE + NOTES_SIZE) % NOTES_SIZE;
	return NOTES[i];
}

int calc_offset(float freq){
	return (int)round(NOTES_SIZE*log2(freq/REFERENCE_FREQ));
}
