#include "music.h"
const char* NOTES[NOTES_SIZE] = {"A", "Bb", "B", "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab"};

const char* get_note_string(float freq){
	int offset = calc_offset(freq);
	int i = (offset % NOTES_SIZE + NOTES_SIZE) % NOTES_SIZE;
	return NOTES[i];
}

int calc_offset(float freq){
	return (int)round(NOTES_SIZE*log2(freq/REFERENCE_FREQ));
}


// metodos para modificar ou o buffer de audio ou o buffer apos fft
void apply_hann_window(float* data, int size) {
	for (int i = 0; i < size; i++) {
		float multiplier = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (size - 1)));
		data[i] *= multiplier;
	}
}

float harmonic_product_spectrum(float* hps, cplx* buff, int size){
	float max_mag = 0.0;
	for (int i = 0; i < size / HPS_FACTOR; i++){
		hps[i] = 1;
		for (int j = 1; j <= HPS_FACTOR; j++){
			cplx bin = buff[i*j];
			float mag = creal(bin)*creal(bin) + cimag(bin)*cimag(bin);
			max_mag = fmax(max_mag, mag);
			
			hps[i] *= mag;
		}
	}
	return max_mag;
}
		
float find_fundamental_freq(float* buff, int size){
	float bin, max = 0;
	for (int i = 0; i < size; i++){
		if (buff[i] > max){
			max = buff[i];
			bin = i;
		}
	}

	if (bin > 0 && bin < size-1){
		float lbin = buff[(int)bin-1];
		float mbin = buff[(int)bin];
		float rbin = buff[(int)bin+1];

		bin = bin + 0.5*(lbin - rbin) / (lbin - 2*mbin + rbin);
	}
	return bin;
}
