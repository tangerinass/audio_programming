#ifndef MUSIC_CONST_
#define MUSIC_CONST_

#include "fft.h"
#include <math.h>

// frequencia de A a 440Hz
#define REFERENCE_FREQ 440
#define NOTES_SIZE 12

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 2048
#define CHANNELS 1
#define HPS_FACTOR 5

#define MIN_MAG 20
#define MIN_NOTE 10

extern const char* NOTES[NOTES_SIZE];

const char* get_note_string(float freq);

int calc_offset(float freq);

float harmonic_product_spectrum(float* hps, cplx* buff, int size);

void apply_hann_window(float* data, int size);

float find_fundamental_freq(float* buff, int size);
#endif
