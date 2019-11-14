// #include "Arduino.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "board.h"

#include "audio_common.h"
#include "audio_pipeline.h"

#include "i2s_stream.h"
#include "raw_stream.h"

#include "esp_peripherals.h"
#include "periph_button.h"

#include "filter_resample.h"

#include "sound_project.h"

#include "esp_dsp.h"

#include "DTWdist.h"
// #include "esp32-hal-log.h"


static const char *TAG = "sound_project";
static const char *EVENT_TAG = "board";

//AUDIO CONFIGURATION
#define AUDIOCHUNKSIZE 480
#define SAMPLERATE 16000
#define DOWNSAMPLE_RATE 16000
#define OUTPUTCHANNELS 1

//MFCC and DTW
#define MFCC_COEFF_COUNT 12
#define DTW_WARPING_WINDOW 100
#define MFCC_LENGTH 600

//INPUT SOURCES
#define MIC AUDIO_HAL_CODEC_MODE_ENCODE
#define LINEIN AUDIO_HAL_CODEC_MODE_LINE_IN
#define SOURCE LINEIN



sound_buffer *audio_buffer;



void app_main() {
    audio_buffer = (sound_buffer *) heap_caps_malloc(sizeof(sound_buffer), MALLOC_CAP_8BIT);
    audio_buffer->filledSamples = 0;

    sound_input_struct_t *soundInput = setupRecording(SAMPLERATE, SOURCE, OUTPUTCHANNELS);
    printf("Free Size %u\r\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));
    
    xTaskCreatePinnedToCore(readSignal,
                "readSignal",
                8192*10,
                (void*)soundInput,
                1,
                NULL,
                0);
                
    //xTaskCreate(printMFCC, "printMFCC", 8192, NULL, 2, NULL);
    printf("Free Size %u\r\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));

}

float int16ToFloatScaling(int16_t toNormalize) {
    //https://stats.stackexchange.com/questions/70801/how-to-normalize-data-to-0-1-range

            // a = (1 - -1)/(32767 - -32768)
            // b = 1 - a * 32767
            // a                                        // b
    return 3.0518043793392844e-05 * toNormalize + 1.5259021896696368e-05;

}

csf_float * calculateHamming(int windowLength) {
    csf_float *hammWindow = (csf_float *) malloc(sizeof(csf_float) * windowLength);
        for (int32_t i = 0; i < windowLength; i++) {
            hammWindow[i] = 0.54 - 0.46 * cosf((2 * M_PI * i)/ (windowLength - 1));
        }
    return hammWindow;
}

void printMFCC(csf_float **aMFCC, int32_t frames, int32_t coeffNum) {
    int splitCounter = 0;
    for (int i = 0; i < ((frames * coeffNum)); i++) {
        if((i%coeffNum) != 0) printf("%f ", *(*(aMFCC)+ i)), splitCounter++;;
        if (splitCounter == 12) splitCounter = 0, printf("\n");
    }
    printf("\n");
}

void printSignal(int16_t *signal) {
        for (int i = 0; i < AUDIOCHUNKSIZE; i++) {
            printf("%hi ", signal[i]);
        }
        printf("\n");
}


void readSignal(void *soundInput) {
    sound_input_struct_t *soundSettings = (sound_input_struct_t *) soundInput;

    unsigned int aSignalLen = SAMPLE_BUFFER_SIZE;
    int aSampleRate = DOWNSAMPLE_RATE;
    csf_float aWinLen = 0.03; // 25ms
    csf_float aWinStep = 0.03; // 10ms per window movement
    int aNCep = 13; // Num of coefficents
    int aNFilters = 26; // Number of filters
    int aNFFT = 512; // FFT size
    int aLowFreq = 0; // Lowest frequency 
    int aHighFreq = 0; // If <= low, then it is samplerate / 2; Nyquist
    csf_float aPreemph = 0.97; 
    int aCepLifter = 22;
    int aAppendEnergy = 1; // Add spectral energy to aMFCC[0]
    csf_float* aWinFunc = NULL; //calculateHamming(aSampleRate * aWinLen); // Windowing function should use hamming / hanning later TODO
    int frames = 0; // Frame counter

    csf_float **aMFCC = (csf_float **) malloc(sizeof(csf_float *));

    float blenderDistance = 0;
    float humveeDistance = 0;
    float gunDistance = 0;
    float waterDistance = 0;

    for(;;) {
        // Read audio samples from pipeline
        raw_stream_read(soundSettings->raw_read, (char *)soundSettings->buffer, AUDIOCHUNKSIZE * sizeof(int16_t));


        // Copy read samples from buffer to bigger buffer
        memcpy(&audio_buffer->sampleBuffer[audio_buffer->filledSamples], soundSettings->buffer, sizeof(int16_t) * AUDIOCHUNKSIZE);
        // Update buffer counter
        audio_buffer->filledSamples += AUDIOCHUNKSIZE;

        // If buffer is full
        if (audio_buffer->filledSamples == SAMPLE_BUFFER_SIZE) {

            // Calculate MFCC coefficents on long buffer
            frames = csf_mfcc(audio_buffer->sampleBuffer,
                            aSignalLen, 
                            aSampleRate, 
                            aWinLen, 
                            aWinStep, 
                            aNCep, 
                            aNFilters, 
                            aNFFT, 
                            aLowFreq, 
                            aHighFreq, 
                            aPreemph, 
                            aCepLifter, 
                            aAppendEnergy, 
                            aWinFunc, 
                            aMFCC);

            // Calculate distances between MFCCs // frames * MFCC_COEFF_COUNT
            unsigned start = xthal_get_ccount(); //benchmarking
            blenderDistance = calculateDistance(*aMFCC, blender_mfcc , frames * MFCC_COEFF_COUNT, MFCC_LENGTH, DTW_WARPING_WINDOW);
            humveeDistance = calculateDistance(*aMFCC, humvee_mfcc, frames * MFCC_COEFF_COUNT, MFCC_LENGTH, DTW_WARPING_WINDOW);
            gunDistance = calculateDistance(*aMFCC, gun2_mfcc, frames * MFCC_COEFF_COUNT, MFCC_LENGTH, DTW_WARPING_WINDOW);
            waterDistance = calculateDistance(*aMFCC, water_mfcc, frames * MFCC_COEFF_COUNT, MFCC_LENGTH, DTW_WARPING_WINDOW);
            unsigned end = xthal_get_ccount(); //benchmarking
            printf("%d\n", end - start);
            printf("Blender: %f\tHumvee: %f\tGun: %f\tWater: %f\n", blenderDistance, humveeDistance, gunDistance, waterDistance);

            // Free allocated MFCC buffer from csf_mfcc function
            free(*aMFCC);



            
            // Buffer is used.
            audio_buffer->filledSamples = 0;
        }

    }
    free(aMFCC);
}

sound_input_struct_t *setupRecording(int sampleRate, audio_hal_codec_mode_t source, int32_t outputChannels)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set(EVENT_TAG, ESP_LOG_INFO);


    sound_input_struct_t *soundInput = (sound_input_struct_t *)malloc(sizeof(sound_input_struct_t));
    int16_t *buff = (int16_t *)heap_caps_malloc(AUDIOCHUNKSIZE * sizeof(int16_t), MALLOC_CAP_8BIT);
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_reader, filter, raw_read;

    if (NULL == buff) {
        ESP_LOGE(EVENT_TAG, "Memory allocation failed!");
        return NULL;
    }

    ESP_LOGI(EVENT_TAG, "[ 1 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, source, AUDIO_HAL_CTRL_START);

    ESP_LOGI(EVENT_TAG, "[ 2.0 ] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(EVENT_TAG, "[ 2.1 ] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.i2s_config.sample_rate = sampleRate;
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(EVENT_TAG, "[ 2.2 ] Create filter to resample audio data");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = sampleRate;
    rsp_cfg.src_ch = 2;
    rsp_cfg.dest_rate = DOWNSAMPLE_RATE;
    rsp_cfg.dest_ch = outputChannels;
    rsp_cfg.type = AUDIO_CODEC_TYPE_ENCODER;
    filter = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(EVENT_TAG, "[ 2.3 ] Create raw to receive data");
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 16 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    raw_read = raw_stream_init(&raw_cfg);

    ESP_LOGI(EVENT_TAG, "[ 3 ] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, raw_read, "raw");

    ESP_LOGI(EVENT_TAG, "[ 4 ] Link elements together [codec_chip]-->i2s_stream-->filter-->raw-->[SR]");
    audio_pipeline_link(pipeline, (const char *[]) {"i2s", "filter", "raw"}, 3);
    // audio_pipeline_link(pipeline, (const char *[]) {"i2s", "raw"}, 2);

    ESP_LOGI(EVENT_TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    soundInput->pipeline = pipeline;
    soundInput->i2s_stream_reader = i2s_stream_reader;
    soundInput->filter = filter;
    soundInput->raw_read = raw_read;
    soundInput->buffer = buff;

    return soundInput;
}

void cleanupRecording(sound_input_struct_t *soundInput)
{
    ESP_LOGI(EVENT_TAG, "[ 6 ] Stop audio_pipeline");

    audio_pipeline_terminate(soundInput->pipeline);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(soundInput->pipeline);

    audio_pipeline_unregister(soundInput->pipeline, soundInput->raw_read);
    audio_pipeline_unregister(soundInput->pipeline, soundInput->i2s_stream_reader);
    audio_pipeline_unregister(soundInput->pipeline, soundInput->filter);

    /* Release all resources */
    audio_pipeline_deinit(soundInput->pipeline);
    audio_element_deinit(soundInput->raw_read);
    audio_element_deinit(soundInput->i2s_stream_reader);
    audio_element_deinit(soundInput->filter);

    free(soundInput->buffer);
    soundInput->buffer = NULL;
    free(soundInput);
    soundInput = NULL;
}
