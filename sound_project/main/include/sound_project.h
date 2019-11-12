#ifndef SOUND_PROJECT_H
#define SOUND_PROJECT_H 

#include "calcDTW_C.h"

#include "c_speech_features.h"

typedef struct {
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_reader;
    audio_element_handle_t filter;
    audio_element_handle_t raw_read;
    int16_t *buffer;
} sound_input_struct_t;


sound_input_struct_t *setupRecording(int sampleRate, audio_hal_codec_mode_t source, int32_t outputChannels);
void cleanupRecording(sound_input_struct_t *soundInput);
void readSignal(void *soundInput);
csf_float * calculateHamming(int windowLength);

#endif