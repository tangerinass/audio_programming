#define MINIAUDIO_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include "miniaudio.h"

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
        MA_COPY_MEMORY(temp_write, pOutput, size * bytes_per_frame);
        ma_pcm_rb_commit_write(pDevice->pUserData, size);
	
}

int main(int argc, char* argv[]) {
	// quero fazr um ring buffer que le o audio do meu device de captura e enquanto faço isso
	// passo uma fft pelo buffer para obter (se funfar) a nota correspondete ao audio	
	ma_result result;

	ma_pcm_rb rbuffer;
	ma_device_config deviceConfig;
	ma_device device;
	
	void *temp_read;
	
	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.capture.format   = ma_format_f32;
	deviceConfig.capture.channels = CHANNELS;
	deviceConfig.sampleRate       = SAMPLE_RATE;
	deviceConfig.dataCallback     = data_callback;
	deviceConfig.pUserData        = &rbuffer;
	
	result = ma_pcm_rb_init(ma_format_f32, CHANNELS, BUFFER_SIZE, NULL, NULL, &rbuffer);
                           
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
	char c = getchar();
	while (c != 'q'){
		ma_uint32 size = BUFFER_SIZE * ma_get_bytes_per_frame(deviceConfig.capture.format, deviceConfig.capture.channels);
		ma_pcm_rb_acquire_read(&rbuffer, &size ,&temp_read);
		
		for (ma_uint32 i = 0; i < BUFFER_SIZE; ++i){
			printf("%lf\n",((float*)(temp_read))[i]);
		}

		ma_pcm_rb_commit_read(&rbuffer, size);
		c = getchar();
	}

	// fazer aqui o meu programa
	ma_device_uninit(&device);
	return 0;
}		
