#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "miniaudio.h"
#include <math.h>
#include <pthread.h>

#define MAXNOTES 1

#define ERRSETUPAUDIO   "erro ao tentar dar setup ao audio"
#define ERRPLAYAUDIO    "erro ao tentar tocar o audio"
#define ERRSTOPAUDIO    "erro ao tentar parar o audio"

#define SAMPLE_RATE 48000

pthread_mutex_t NOTE_LOCK;

typedef enum Note {
    Inv,
    C, 
    Db, 
    D, 
    Eb, 
    E, 
    F, 
    Gb, 
    G,
    Ab,
    A, 
    Bb,
    B,
} Note;

float NOTE_FREQS[] = {0, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88};

typedef struct Info {
    ma_device_config audio_config;
    Note note;
    double t;
} Info;


float get_freq(Note note){
    return NOTE_FREQS[note];
}

char* get_note_string(Note note){
    switch (note)
    {
        case A: 
            return "A"; 
        case Ab: 
            return "Ab";
        case B: 
            return "B"; 
        case Bb: 
            return "Bb"; 
        case C:
            return "C";
        case D: 
            return "D";
        case Db: 
            return "Db"; 
        case E: 
            return "E"; 
        case Eb: 
            return "Eb"; 
        case F: 
            return "F"; 
        case G: 
            return "G"; 
        case Gb: 
            return "Gb"; 
        default:
            return "";
    }
}

Note get_note_enum(char in){
    if (in == 'z') return C;
    if (in == 's') return Db;
    if (in == 'x') return D;
    if (in == 'd') return Eb;
    if (in == 'c') return E;
    if (in == 'v') return F;
    if (in == 'g') return Gb;
    if (in == 'b') return G;
    if (in == 'h') return Ab;
    if (in == 'n') return A;
    if (in == 'j') return Bb;
    if (in == 'm') return B;
    return Inv;
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    pthread_mutex_lock(&NOTE_LOCK);

    ma_waveform pSineWave;
    ma_waveform_config sineWaveConfig;
    
    Note data = (Note)pDevice->pUserData;

    sineWaveConfig = ma_waveform_config_init(pDevice->playback.format, pDevice->playback.channels, pDevice->sampleRate, ma_waveform_type_sawtooth, 0.1, get_freq(data));
    
    ma_waveform_init(&sineWaveConfig, &pSineWave);

    ma_waveform_read_pcm_frames(&pSineWave, pOutput, frameCount, NULL);

    pthread_mutex_unlock(&NOTE_LOCK);

    (void)pInput;
}

int play(Info *info){
    ma_device device;
    char c;
    Note n;

    if (ma_device_init(NULL, &info->audio_config, &device) != MA_SUCCESS) {
        printf("%s\n", ERRSETUPAUDIO);
        return -1;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("%s\n",ERRPLAYAUDIO);
        ma_device_uninit(&device);
        return -1;
    }

    printf("carregue '.' para parar\n");

    system ("/bin/stty raw");
    while((c=getchar())!= '.') {
        pthread_mutex_lock(&NOTE_LOCK);
        info->note = get_note_enum(c);
        pthread_mutex_unlock(&NOTE_LOCK);
    }
    system ("/bin/stty cooked");

    ma_device_uninit(&device);
    printf("audio desligado\n");
    return 0;
}

void end(Info *info){
    pthread_mutex_destroy(&NOTE_LOCK);
    exit(0);
}


int main(int argc, char* argv[]) {
    char buf[BUFSIZ];
    Info program_info;
    
    program_info.note;

    program_info.audio_config = ma_device_config_init(ma_device_type_playback);
    program_info.audio_config.sampleRate = SAMPLE_RATE;
    program_info.audio_config.playback.format = ma_format_f32;
    program_info.audio_config.playback.channels = 2;
    program_info.audio_config.dataCallback = data_callback;
    program_info.audio_config.pUserData = (void*)program_info.note;

    pthread_mutex_init(&NOTE_LOCK, NULL);
    
    while (fgets(buf, BUFSIZ, stdin))
    {
        switch (buf[0])
        {
        case 'p': play(&program_info);break;
        case 'q': end(&program_info);
        default: break;
        }
    }

    return 0;
}