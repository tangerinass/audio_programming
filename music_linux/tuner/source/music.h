#ifndef MUSIC_CONST_
#define MUSIC_CONST_

// frequencia de A a 440Hz
#define REFERENCE_FREQ 440
#define NOTES_SIZE 12

extern const char* NOTES[NOTES_SIZE];

const char* get_note_string(float freq);

int calc_offset(float freq);

#endif
