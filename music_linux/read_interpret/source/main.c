#define MINIAUDIO_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include "../miniaudio.h"

#include "fft.h"

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 512
#define CHANNELS 1

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{	
	// escrevo no ring buffer frameCount frames
	ma_uint32 size = frameCount;
    	ma_uint32 bytes_per_frame = ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels);
	
	void *temp_write;
	
	ma_pcm_rb_acquire_write(pDevice->pUserData, &size, &temp_write);
        MA_COPY_MEMORY(temp_write, pInput, size * bytes_per_frame);
        ma_pcm_rb_commit_write(pDevice->pUserData, size);
	
}

int main(int argc, char* argv[]) {
	// quero fazr um ring buffer que le o audio do meu device de captura e enquanto faço isso
	// passo uma fft pelo buffer para obter (se funfar) a nota correspondete ao audio	
	ma_result result;

	ma_pcm_rb rbuffer;
	ma_device_config deviceConfig;
	ma_device device;
	
	float *temp_read;

	cplx *fft_buff;
	
	deviceConfig = ma_device_config_init(ma_device_type_capture);
	deviceConfig.capture.format   = ma_format_f32;
	deviceConfig.capture.channels = CHANNELS;
	deviceConfig.sampleRate       = SAMPLE_RATE;
	deviceConfig.dataCallback     = data_callback;
	deviceConfig.pUserData        = &rbuffer;
	
	// para evitar overflow fazr o buffer dobro do tamanho
	result = ma_pcm_rb_init(ma_format_f32, CHANNELS, BUFFER_SIZE * 2, NULL, NULL, &rbuffer);
                           
        if (result != MA_SUCCESS) {
                printf("Failed to to create ring buffer.\n");
                return -3;
        }
	
	result = ma_device_init(NULL, &deviceConfig, &device);
	if (result != MA_SUCCESS) {
                ma_pcm_rb_uninit(&rbuffer);
		printf("Failed to initialize capture device.\n");
		return -2;
	}

	result = ma_device_start(&device);
	
	if (result != MA_SUCCESS) {
                ma_pcm_rb_uninit(&rbuffer);
		ma_device_uninit(&device);
		printf("Failed to start device.\n");
		return -3;
	}

	// n estou a conseguir ler o buffer, n sei se o buffer esta vazio ou se algo esta a correr mal sem ser isso
	fft_buff = malloc(sizeof(cplx)*BUFFER_SIZE);

	while (1){
		ma_uint32 frames;
		
		frames = BUFFER_SIZE;

		ma_pcm_rb_acquire_read(&rbuffer, &frames ,(void**)&temp_read);
		
		for (int i = 0; i < BUFFER_SIZE; i++){
			fft_buff[i] = CMPLX(temp_read[i] * 1000.0, 0.0);
		}
		ma_pcm_rb_commit_read(&rbuffer, frames);
		
		// aqui faço o meu fft e depois encontro a frequeuncia da sample
		//printf("LEVEL:(%lf)\n",creal(fft_buff[0]));
		fft(fft_buff, BUFFER_SIZE);
		int ind = 0;
		float mag = -1.0f;

		for (int i = 0; i < BUFFER_SIZE / 2; i++){
			int new_mag = creal(fft_buff[i]) * creal(fft_buff[i]) + 
				cimag(fft_buff[i]) *  cimag(fft_buff[i]);
			if (new_mag > mag){
				ind = i;
				mag = new_mag;
			}

		}
		
		float freq = (float)ind * SAMPLE_RATE / (float)BUFFER_SIZE;
		printf("freq:(%lf)\n", freq);
		ma_sleep(10);
	}

	free(fft_buff);
	ma_device_uninit(&device);
	return 0;
}		
