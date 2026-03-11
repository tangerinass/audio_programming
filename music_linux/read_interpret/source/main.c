#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio.h"
#include "fft.h"

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 2048
#define CHANNELS 1

/*
	While MSVC does provide a <complex.h> header, it does not implement complex numbers as native types, 
	but as structs, which are incompatible with standard C complex types and do not support the +, -, *, / operators.

	Ou seja para windows isto vai ter de ir assim.. n vi como interfere para linux mas acho q n ha stress
*/ 
#define CMPLX(x, y) ((double complex)((double)(x) + I * (double)(y)))

void apply_hann_window(float* data, int size) {
    for (int i = 0; i < size; i++) {
        float multiplier = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (size - 1)));
        data[i] *= multiplier;
    }
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
    float processing_buff[BUFFER_SIZE];

    printf("Listening... Press Ctrl+C to stop.\n");

	// main loop cosiste em ver se consigo encher o meu buffer todo, se puder ent enho o meu buffer
	// depois de encher o buffer aplico uma hann windowing algoritmo (pucxei do chat) q so basicamente faz com que haja menos descont
	// asseguir aplico ua fft na sample e dps vejo qual a freq predominante na sample e dou print
	// "simples"
	// o gemini deu ganda save com a hann window
    while (1) {
        ma_uint32 available;
        available = ma_pcm_rb_available_read(&rbuffer);

        if (available >= BUFFER_SIZE) {
            ma_uint32 framesRead = BUFFER_SIZE;
            float* pReadBuffer;

            if (ma_pcm_rb_acquire_read(&rbuffer, &framesRead, (void**)&pReadBuffer) == MA_SUCCESS) {
                memcpy(processing_buff, pReadBuffer, sizeof(float) * BUFFER_SIZE);
                ma_pcm_rb_commit_read(&rbuffer, framesRead);

                for (int i = 0; i < BUFFER_SIZE; i++) 
                    fft_buff[i] = CMPLX(processing_buff[i], 0.0);

                fft(fft_buff, BUFFER_SIZE);

                // ver onde esta o pico de amp
                int best_bin = 1;
                double max_mag_sq = -1.0;
                for (int i = 1; i < BUFFER_SIZE / 2; i++) {
                    double mag_sq = creal(fft_buff[i])*creal(fft_buff[i]) + cimag(fft_buff[i])*cimag(fft_buff[i]);
                    if (mag_sq > max_mag_sq) {
                        max_mag_sq = mag_sq;
                        best_bin = i;
                    }
                }

                if (max_mag_sq > 0.001) { // ignorar ruido 
                    float freq = (float)best_bin * SAMPLE_RATE / BUFFER_SIZE;
                    printf("Freq: %7.2f Hz\r", freq);
                    fflush(stdout);
                }
            }
        }
        ma_sleep(10);
    }

    free(fft_buff);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
    return 0;
}