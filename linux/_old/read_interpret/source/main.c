#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio.h"
#include "fft.h"
#include "music.h"

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 2048
#define CHANNELS 1
#define HPS_FACTOR 5
/*
   While MSVC does provide a <complex.h> header, it does not implement complex numbers as native types, 
   but as structs, which are incompatible with standard C complex types and do not support the +, -, *, / operators.

   Ou seja para windows isto vai ter de ir assim.. n vi como interfere para linux mas acho q n ha stress
*/

// por enquanto so vou focar me em linux
//#define CMPLX(x, y) ((double complex)((double)(x) + I * (double)(y)))


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

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	ma_pcm_rb* pRB = (ma_pcm_rb*)pDevice->pUserData;
	ma_uint32 framesToWrite = frameCount;
	void* pWriteBuffer;

	if (ma_pcm_rb_acquire_write(pRB, &framesToWrite, &pWriteBuffer) == MA_SUCCESS) {
		MA_COPY_MEMORY(pWriteBuffer, pInput, framesToWrite * ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels));
		ma_pcm_rb_commit_write(pRB, framesToWrite);
	}
}

int main() {
	ma_result result;
	ma_context context;
	ma_device_info* pCaptureInfos;
	ma_uint32 captureCount;
	ma_uint32 selection = 0;

	ma_context_init(NULL, 0, NULL, &context);
	ma_context_get_devices(&context, NULL, NULL, &pCaptureInfos, &captureCount);

	// escolher o microfone a ser usado
	printf("Select Input Device:\n");
	
	for (ma_uint32 i = 0; i < captureCount; i++) printf("[%d] %s\n", i, pCaptureInfos[i].name);
	
	printf("> ");
	scanf("%u", &selection);

	// fazr ringbuffer, basicamente acesso mais low level possivel e sem locks ao buffer
	ma_pcm_rb rbuffer;
	ma_pcm_rb_init(ma_format_f32, CHANNELS, BUFFER_SIZE * 4, NULL, NULL, &rbuffer);

	ma_device_config config = ma_device_config_init(ma_device_type_capture);
	config.capture.pDeviceID = &pCaptureInfos[selection >= captureCount ? 0 : selection].id;
	config.capture.format = ma_format_f32;
	config.capture.channels = CHANNELS;
	config.sampleRate = SAMPLE_RATE;
	config.dataCallback = data_callback;
	config.pUserData = &rbuffer;

	ma_device device;
	
	if (ma_device_init(&context, &config, &device) != MA_SUCCESS) return -1;
	ma_device_start(&device);

	cplx* fft_buff = malloc(sizeof(cplx) * BUFFER_SIZE);
	float* processing_buff = malloc(sizeof(float) * BUFFER_SIZE);

	printf("Listening... Press Ctrl+C to stop.\n");

	// main loop cosiste em ver se consigo encher o meu buffer todo, se puder ent enho o meu buffer
	// depois de encher o buffer aplico uma hann windowing algoritmo (pucxei do chat) q so basicamente faz com que haja menos inconsistencias
	// finalmente aplico o meu algoritmo de Harmonic Produt Spectrum que 
	while (1) {
		ma_uint32 available;
		available = ma_pcm_rb_available_read(&rbuffer);

		if (available >= BUFFER_SIZE) {
			ma_uint32 framesRead = BUFFER_SIZE;
			float* pReadBuffer;

			if (ma_pcm_rb_acquire_read(&rbuffer, &framesRead, (void**)&pReadBuffer) == MA_SUCCESS) {
				memcpy(processing_buff, pReadBuffer, sizeof(float) * BUFFER_SIZE);
				ma_pcm_rb_commit_read(&rbuffer, framesRead);
				
				apply_hann_window(processing_buff, BUFFER_SIZE);

				for (int i = 0; i < BUFFER_SIZE; i++) 
					fft_buff[i] = CMPLX(processing_buff[i], 0.0);

				fft(fft_buff, BUFFER_SIZE);

				float mag = harmonic_product_spectrum(processing_buff, fft_buff, BUFFER_SIZE);
				
				double freq = find_fundamental_freq(processing_buff, BUFFER_SIZE) * device.sampleRate / BUFFER_SIZE;
				
				if (mag > 10) printf("Freq: %lf, Note: %s\n", freq, get_note_string(freq)); 

			}
		}
		ma_sleep(10);
	}

	free(fft_buff);
	ma_device_uninit(&device);
	ma_context_uninit(&context);
	return 0;
}
