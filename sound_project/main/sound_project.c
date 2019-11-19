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
#include "audio_event_iface.h"

#include "i2s_stream.h"
#include "raw_stream.h"
#include "wav_decoder.h"

#include "esp_peripherals.h"
#include "periph_button.h"
#include "fatfs_stream.h"

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
#define DTW_WARPING_WINDOW 30
#define LBK_WINDOW DTW_WARPING_WINDOW*2

#define MFCC_LENGTH 300

//INPUT SOURCES
#define MIC AUDIO_HAL_CODEC_MODE_ENCODE
#define BOTH AUDIO_HAL_CODEC_MODE_BOTH
#define LINEIN AUDIO_HAL_CODEC_MODE_LINE_IN
#define SOURCE MIC



sound_buffer *audioBuffer;
audio_streams *streams;
detection_model *models;
const char *labels[] = {"water", "humvee","gun", "blender", "blender2"};
const char *responses[] = {"/sdcard/blender.wav","/sdcard/humvee.wav","/sdcard/gun.wav", "/sdcard/water.wav"};

void app_main() {
    audioBuffer = (sound_buffer *) heap_caps_malloc(sizeof(sound_buffer), MALLOC_CAP_8BIT);
    streams = (audio_streams *) heap_caps_malloc(sizeof(audio_streams), MALLOC_CAP_8BIT);
    models = (detection_model *) heap_caps_malloc(sizeof(detection_model), MALLOC_CAP_8BIT);
    models->detectedIndex = -1;
    models->mfccsInModel = MFCCS_IN_MODEL;
    models->mfccs[0] = water_mfcc;
    models->mfccs[1] = humvee_mfcc;
    models->mfccs[2] = gun2_mfcc;
    // models->mfccs[3] = blender_mfcc;
    models->mfccs[3] = blender2_mfcc;
    audioBuffer->filledSamples = 0;

    
    setupBoard(SOURCE);
    sound_output_struct_t *soundOutput = setupPlayer();
    soundOutput->playing = 0;
    sound_input_struct_t *soundInput = setupRecording(SAMPLERATE, OUTPUTCHANNELS);

    streams->inputStream = soundInput;
    streams->outputStream = soundOutput;

    printf("Free Size %u\r\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));
    
    xTaskCreatePinnedToCore(readSignal,
                "readSignal",
                8192*8,
                (void*)soundInput,
                1,
                NULL,
                0);
                
    printf("Free Size %u\r\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));



    // while(1) {
    //     audio_event_iface_msg_t msg;
    //     audio_event_iface_listen(evt, &msg, 0);
    //     if ((int) msg.data == get_input_play_id() && !soundOutput->playing) {
    //         printf("Play button pressed\n");
    //         // audio_pipeline_stop(soundOutput->pipeline);
    //         soundOutput->playing = 1;
    //         // audio_element_set_uri(soundOutput->fatfs_reader, "/sdcard/gun1.wav");
    //         audio_pipeline_run(soundOutput->pipeline);
    //     }
    //     if ((int) msg.data == get_input_set_id() && soundOutput->playing) {
    //         printf("Stop button pressed\n");
    //         stopPlaying(soundOutput);
    //         audio_element_set_uri(soundOutput->fatfs_reader, "/sdcard/test.wav");
    //         soundOutput->playing = 0;
    //     }
    //     // memset(&msg, 0, sizeof(audio_event_iface_msg_t));
    //     vTaskDelay(100 / portTICK_PERIOD_MS);
    // }

}

void stopPlaying(sound_output_struct_t *soundOutput) {
        audio_pipeline_stop(soundOutput->pipeline);
        audio_pipeline_wait_for_stop(soundOutput->pipeline);
        audio_element_reset_state(soundOutput->decoder);
        audio_element_reset_state(soundOutput->i2s_stream_writer);
        audio_pipeline_reset_ringbuffer(soundOutput->pipeline);
        audio_pipeline_reset_items_state(soundOutput->pipeline);
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

uint32_t FindNearestMatch(detection_model *models) {
    float bestSoFar = INFINITY;
    float LBDist = INFINITY;
    float trueDist = INFINITY;
    float sumOfDifferences = 0;
    int32_t modelsCompared = 0;
    for (int32_t i = 0; i < models->mfccsInModel; i++)
    {
        LBDist = LBKeogh(models->currentMFCC, models->mfccs[i], MFCC_LENGTH, LBK_WINDOW);
        printf("LBKeogh distance:%f at model %d ", LBDist, i);
        if (LBDist < bestSoFar) { 
            trueDist = calculateDistance(models->currentMFCC, models->mfccs[i], MFCC_LENGTH, MFCC_LENGTH, DTW_WARPING_WINDOW);
            printf("DTW distance:%f at model %d ", trueDist, i);
            sumOfDifferences += trueDist, modelsCompared++;
        }
        if (trueDist < bestSoFar) bestSoFar = trueDist, models->detectedIndex = i;
        printf("\n");
    }
    models->differences = sumOfDifferences / modelsCompared - bestSoFar;

    return models->detectedIndex;
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
    float power = 0;

    csf_float **aMFCC = (csf_float **) malloc(sizeof(csf_float *));

    for(;;) {
        // Read audio samples from pipeline
        raw_stream_read(soundSettings->raw_read, (char *)soundSettings->buffer, AUDIOCHUNKSIZE * sizeof(int16_t));

        // printSignal(soundSettings->buffer);.

        // Copy read samples from buffer to bigger buffer
        memcpy(&audioBuffer->sampleBuffer[audioBuffer->filledSamples], soundSettings->buffer, sizeof(int16_t) * AUDIOCHUNKSIZE);
        // Update buffer counter
        audioBuffer->filledSamples += AUDIOCHUNKSIZE;

        // If buffer is full
        if (audioBuffer->filledSamples == SAMPLE_BUFFER_SIZE) {

            // Calculate MFCC coefficents on long buffer
            frames = csf_mfcc(audioBuffer->sampleBuffer,
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


            models->currentMFCC = dumpFirstCoeffAndCalcPower(*aMFCC, frames, aNCep, &power);
            // printf("%f\n", power);

            // zNormalizeMfcc(models->currentMFCC, 12);

            // Calculate distances between MFCCs // frames * MFCC_COEFF_COUNT
            // unsigned start = xthal_get_ccount(); //benchmarking

            if ((power > 10 && SOURCE == LINEIN) || (power > 13 && SOURCE == MIC)) {
                
                printf("\n");
                printf("Best match in model is labeled: %s\n", labels[FindNearestMatch(models)]);
                // printf("%f\n", models->differences);
                // playAnswer();
            }
            else printf("No sound detected on spectrum\n");

            // unsigned end = xthal_get_ccount(); //benchmarking
            // printf("Clk cycles: %d\n", end - start);

            // Free allocated MFCC buffer from csf_mfcc function
            free(*aMFCC);
            free(models->currentMFCC);
            // printf("Free memory: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));



            
            // Buffer is used.
            audioBuffer->filledSamples = 0;
        }

    }
    free(aMFCC);
}

float * dumpFirstCoeffAndCalcPower(float * input, int frames, uint32_t numOfCoeff, float * power) {
    float * output = (float *) malloc(sizeof(float) * (frames * (numOfCoeff - 1)));


    for (int32_t inputIndex = 1, outputIndex = 0; inputIndex < frames * numOfCoeff; inputIndex += numOfCoeff, outputIndex += (numOfCoeff - 1)) {
        *power += input[inputIndex - 1];
        memcpy(&output[outputIndex], &input[inputIndex], sizeof(float) * (numOfCoeff - 1));
    }

    *power /= frames;

    return output;
}

audio_event_iface_handle_t setupBoard(audio_hal_codec_mode_t source) {
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set(EVENT_TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "[ 1 ] Initialize the peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[ 1.1 ] Initialize sd card");
    audio_board_sdcard_init(set);

    ESP_LOGI(TAG, "[ 1.2 ] Initialize keys");
    audio_board_key_init(set);

    ESP_LOGI(EVENT_TAG, "[ 1.3 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, source, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[ 1.4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[ 1.5 ] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    return evt;
}

void playAnswer(void) {
    if (streams->outputStream->playing == 1) {
        audio_pipeline_terminate(streams->outputStream->pipeline);
        streams->outputStream->playing = 0;
    }
    if (!streams->outputStream->playing) {
        streams->outputStream->playing = 1;
        audio_pipeline_reset_ringbuffer(streams->outputStream->pipeline);
        audio_pipeline_reset_elements(streams->outputStream->pipeline);
        audio_pipeline_change_state(streams->outputStream->pipeline, AEL_STATE_INIT);
        audio_element_set_uri(streams->outputStream->fatfs_reader, responses[models->detectedIndex]);
        audio_pipeline_run(streams->outputStream->pipeline);
    }
}

sound_input_struct_t *setupRecording(int sampleRate, int32_t outputChannels)
{
    sound_input_struct_t *soundInput = (sound_input_struct_t *)malloc(sizeof(sound_input_struct_t));
    int16_t *buff = (int16_t *)heap_caps_malloc(AUDIOCHUNKSIZE * sizeof(int16_t), MALLOC_CAP_8BIT);
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_reader, filter, raw_read;

    if (NULL == buff) {
        ESP_LOGE(EVENT_TAG, "Memory allocation failed!");
        return NULL;
    }

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

sound_output_struct_t *setupPlayer(void) {
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t fatfs_stream_reader, i2s_stream_writer, wav_decoder, filter;

    sound_output_struct_t *outputStream = (sound_output_struct_t *) malloc(sizeof(sound_output_struct_t));

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[ * ] Play from sdcard");
    fatfs_stream_cfg_t fatfs_read_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_read_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_read_cfg);

    i2s_stream_cfg_t i2s_sdcard_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_sdcard_cfg.type = AUDIO_STREAM_WRITER;
    i2s_sdcard_cfg.i2s_config.sample_rate = 48000;
    i2s_stream_writer = i2s_stream_init(&i2s_sdcard_cfg);

    ESP_LOGI(TAG, "[3.3] Create wav decoder to decode wav file");
    wav_decoder_cfg_t wav_cfg = DEFAULT_WAV_DECODER_CONFIG();
    wav_decoder = wav_decoder_init(&wav_cfg);

    ESP_LOGI(EVENT_TAG, "[ 2.2 ] Create filter to resample audio data");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = 8000;
    rsp_cfg.src_ch = 1;
    rsp_cfg.dest_rate = 16000;
    rsp_cfg.dest_ch = 1;
    rsp_cfg.type = AUDIO_CODEC_TYPE_ENCODER;
    filter = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, fatfs_stream_reader, "file");
    audio_pipeline_register(pipeline, wav_decoder, "wav");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[3.5] Link it together [sdcard]-->fatfs_stream-->wav_decoder-->i2s_stream-->[codec_chip]");
    audio_pipeline_link(pipeline, (const char *[]) {"file", "wav", "filter", "i2s"}, 4);
    
    ESP_LOGI(TAG, "[3.6] Set up  uri (file as fatfs_stream, wav as wav decoder, and default output is i2s)");

    outputStream->pipeline = pipeline;
    outputStream->decoder = wav_decoder;
    outputStream->fatfs_reader = fatfs_stream_reader;

    return outputStream;
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

float zNormalizeMfcc(float *mfcc, int32_t numOfCoeff) {
    float *means = (float *) heap_caps_calloc(numOfCoeff, sizeof(float), MALLOC_CAP_8BIT);
    float *deviations = (float *) heap_caps_calloc(numOfCoeff, sizeof(float), MALLOC_CAP_8BIT);
    for (int32_t i = 0, coeffCounter = 0; i < MFCC_LENGTH; i++, coeffCounter++) {
        if (coeffCounter == numOfCoeff) coeffCounter = 0;
        means[coeffCounter] += mfcc[i];
        if (i >= (MFCC_LENGTH - numOfCoeff)) means[coeffCounter] /= (MFCC_LENGTH / numOfCoeff); 
    }
    
    for (int32_t i = 0, coeffCounter = 0; i < MFCC_LENGTH; i++, coeffCounter++) {
        if (coeffCounter == numOfCoeff) coeffCounter = 0;
        deviations[coeffCounter] += (mfcc[i] - means[coeffCounter]) * (mfcc[i] - means[coeffCounter]);
        if (i >= (MFCC_LENGTH - numOfCoeff)) {
            deviations[coeffCounter] /= ((MFCC_LENGTH / numOfCoeff)); 
            deviations[coeffCounter] = sqrt(deviations[coeffCounter]);
        }
    }


    for (int32_t i = 0, coeffCounter = 0; i < MFCC_LENGTH; i++, coeffCounter++) {
        if (coeffCounter == numOfCoeff) coeffCounter = 0;
        mfcc[i] = (mfcc[i] - means[coeffCounter]) / deviations[coeffCounter]; 
    }

    free(means);
    free(deviations);

    return 0;
}