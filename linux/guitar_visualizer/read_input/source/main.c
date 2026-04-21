#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <fcntl.h>
#include <signal.h>
#define MINIAUDIO_IMPLEMENTATION

#include "../../../../miniaudio.h"
#include "music.h"
#include "../../common/common.h"

/*
   While MSVC does provide a <complex.h> header, it does not implement complex numbers as native types, 
   but as structs, which are incompatible with standard C complex types and do not support the +, -, *, / operators.

   Ou seja para windows isto vai ter de ir assim.. n vi como interfere para linux mas acho q n ha stress
   */

// por enquanto so vou focar me em linux descomentar isto se for testar em linux
// ver como faço para ter o mm programa poder correr em systemas op dif
//#define CMPLX(x, y) ((double complex)((double)(x) + I * (double)(y)))

static volatile int run = 1;
void sig_handler(int x){
	run = 0;
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

// encontrar a note q aparece mais na amostra
int find_note_mode(int* buff, int size){
	int note, max, set[NOTES_SIZE] = {0};

	for (int i = 0; i < size; i++){
		set[buff[i]]++;
	}

	max = 0;

	for (int i = 0; i < NOTES_SIZE; i++){
		if (set[i] > max){
			max = set[i];
			note = i;
		}
	}

	return note;
}

// TODO: arranjar maneira de poder encontrar o device do amp e correr sem pedir
int main() {
	int ret, fd, ptr = 0;
	int note_buff[MIN_NOTE];

	ma_device device;
	ma_pcm_rb rbuffer;

	cplx* fft_buff = malloc(sizeof(cplx) * BUFFER_SIZE);
	float* processing_buff = malloc(sizeof(float) * BUFFER_SIZE);

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
	ma_pcm_rb_init(ma_format_f32, CHANNELS, BUFFER_SIZE * 4, NULL, NULL, &rbuffer);

	ma_device_config config = ma_device_config_init(ma_device_type_capture);
	config.capture.pDeviceID = &pCaptureInfos[selection >= captureCount ? 0 : selection].id;
	config.capture.format = ma_format_f32;
	config.capture.channels = CHANNELS;
	config.sampleRate = SAMPLE_RATE;
	config.dataCallback = data_callback;
	config.pUserData = &rbuffer;

	if (ma_device_init(&context, &config, &device) != MA_SUCCESS) return -1;
	ma_device_start(&device);

	ma_context_uninit(&context);

	// main loop cosiste em ver se consigo encher o meu buffer todo, se puder ent enho o meu buffer
	// depois de encher o buffer aplico uma hann windowing algoritmo (pucxei do chat) q so basicamente faz com que haja menos inconsistencias
	// finalmente aplico o meu algoritmo de Harmonic Produt Spectrum que faz os produtos dos bin sempre q se repetem, fazendo com que os picos aumentem facilitando a leitura para frequencias mais baixas (aumentar o hps para melhorar leituras a baixas freeq)
	

	mkfifo(pathname, 0666);
	printf("Script started\n");


	// INFO: deamsiados calculos dentro do loop tentar reduzir, talvez o outro programa trata de fazr as medias?? 
	// idk n sei se reduz assim tatno prb n faz diferença

	signal(SIGINT, sig_handler);
	while (run) {
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

				if (mag > MIN_MAG){
					double freq = find_fundamental_freq(processing_buff, BUFFER_SIZE) * device.sampleRate / BUFFER_SIZE;
					int note = calc_offset(freq);

					note_buff[ptr++] = note;

					//printf("note:%s, freq:%lf, offset:%d, array:%d-%s\n", get_note_string(freq), freq, calc_offset(freq), note_buff[ptr-1], NOTES[note_buff[ptr-1]]);

					if (ptr == MIN_NOTE){
						note = find_note_mode(note_buff, MIN_NOTE);

						printf("Sending note: %s\n", NOTES[note]);
						char tmp[MSG_SIZE];
						sprintf(tmp, "%-19d", note); 


						if ( (fd = open(pathname, O_WRONLY)) < 0){
							perror("Failed trying to opene FIFO");
							exit(-1);
						}

						ret = write(fd, tmp, MSG_SIZE);
						close(fd);
						ptr = 0;
					}
				}

			}
		}
		ma_sleep(10);
	}

	printf("Exiting program\n");
	free(fft_buff);
	ma_device_uninit(&device);
	free(processing_buff);
	
	return 0;
}
