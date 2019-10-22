/* Examples of speech recognition with multiple keywords.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

// #include "esp32-hal-log.h"


static const char *TAG = "sound_project";
static const char *EVENT_TAG = "board";

//AUDIO CONFIGURATION
#define AUDIOCHUNKSIZE 4096
#define SAMPLERATE 44100
#define OUTPUTCHANNELS 1
#define SAMPLETIMEINMS (AUDIOCHUNKSIZE * 10000 / SAMPLERATE)

//INPUT SOURCES
#define MIC AUDIO_HAL_CODEC_MODE_ENCODE
#define LINEIN AUDIO_HAL_CODEC_MODE_LINE_IN


bool started = 0;

void app_main()
{
    // initArduino();
    // pinMode(KEY6_PIN, INPUT_PULLUP);
    sound_input_struct_t *soundInput = setupRecording(SAMPLERATE, LINEIN, OUTPUTCHANNELS);
    printf("Free Size %u\r\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));

    // ringbuf_handle_t rb = audio_element_get_output_ringbuf(soundInput->i2s_stream_reader);

    while (1) {

        // vTaskDelay(SAMPLETIMEINMS / portTICK_RATE_MS);
        int bytesRead = raw_stream_read(soundInput->raw_read, (char *)soundInput->buffer, AUDIOCHUNKSIZE * sizeof(int16_t));

        // vTaskDelay(SAMPLETIMEINMS / portTICK_RATE_MS);


            for (int i = 0; i < AUDIOCHUNKSIZE>>1; i++) {
                printf("%hi ", soundInput->buffer[i]);
            }
            printf("\n");

            
            // audio_pipeline_reset_ringbuffer(soundInput->pipeline);
            
        }
}

sound_input_struct_t *setupRecording(int sampleRate, audio_hal_codec_mode_t source, int32_t outputChannels)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set(EVENT_TAG, ESP_LOG_INFO);


    sound_input_struct_t *soundInput = (sound_input_struct_t *)malloc(sizeof(sound_input_struct_t));
    int16_t *buff = (int16_t *)malloc(AUDIOCHUNKSIZE * sizeof(int16_t));
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
    rsp_cfg.dest_rate = sampleRate;
    rsp_cfg.dest_ch = outputChannels;
    rsp_cfg.type = AUDIO_CODEC_TYPE_ENCODER;
    filter = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(EVENT_TAG, "[ 2.3 ] Create raw to receive data");
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 8 * 1024,
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