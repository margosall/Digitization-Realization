#include "Arduino.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "audio_hal.h"

#include "i2s_stream.h"

#include "raw_stream.h"
#include "filter_resample.h"

#include "esp_peripherals.h"
#include "periph_button.h"

#include "sound_project.h"

#include "esp_dsp.h"

#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

static const char *TAG = "record_raw";

#define AUDIO_CHUNKSIZE 4096


extern "C" void app_main() {
    initArduino();
    Serial.begin(921600);
    
    sound_input_struct_t *soundInput = setupMic(41100);

    

    while (1) {
        int i;
        int bytes_read = raw_stream_read((char *)soundInput->buffer, AUDIO_CHUNKSIZE * sizeof(short));
        // printf("%d\n", bytes_read);        
        for (i = 0; i < AUDIO_CHUNKSIZE; i++) {
            Serial.print(soundInput->buffer[i]);
            Serial.print(" ");
        //     Serial.write(soundInput->buffer[i]);
        //     Serial.write(0xAA);
        //     if (soundInput->buffer[i] == 0xAA) {
        //         Serial.write(0xAA);
        //     }
        // }
        vTaskDelay(5);
        Serial.print("\n");
    }


    cleanUpMic(soundInput);

}



sound_input_struct_t *setupMic(int sampleRate) {

    sound_input_struct_t *soundInput = (sound_input_struct_t *)malloc(sizeof(sound_input_struct_t));
    int16_t *buff = (int16_t *)malloc(AUDIO_CHUNKSIZE * sizeof(short));
    
    if (buff == NULL) {
        return NULL;
    }

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_hal_codec_config_t audio_hal_codec_cfg =  AUDIO_HAL_AC101_DEFAULT();
    audio_hal_codec_cfg.adc_input = AUDIO_HAL_ADC_INPUT_ALL;
    audio_hal_handle_t hal = audio_hal_init(&audio_hal_codec_cfg, BOARD);
    audio_hal_ctrl_codec(hal, AUDIO_HAL_CODEC_MODE_LINE_IN, AUDIO_HAL_CTRL_START);


    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_reader, filter, raw_read;


    ESP_LOGI(TAG, "[2.0] Create audio pipeline for recording");
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
    rsp_cfg.dest_ch = 1;
    rsp_cfg.type = AUDIO_CODEC_TYPE_ENCODER;
    filter = rsp_filter_init(&rsp_cfg);


    ESP_LOGI(EVENT_TAG, "[ 2.3 ] Create raw to receive data");
    raw_stream_cfg_t raw_cfg = {
        .type = AUDIO_STREAM_READER,
        .out_rb_size = 8 * 1024,
    };
    raw_read = raw_stream_init(&raw_cfg);

    ESP_LOGI(EVENT_TAG, "[ 3 ] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, raw_read, "raw");

    ESP_LOGI(EVENT_TAG, "[ 4 ] Link elements together [codec_chip]-->i2s_stream-->filter-->raw");

    const char* link_array[] = {"i2s", "filter", "raw"};
    audio_pipeline_link(pipeline, link_array, 3);

    ESP_LOGI(EVENT_TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    soundInput->pipeline = pipeline;
    soundInput->i2s_stream_reader = i2s_stream_reader;
    soundInput->filter = filter;
    soundInput->raw_read = raw_read;
    soundInput->buffer = buff;

    return soundInput;
} 

void cleanUpMic(sound_input_struct_t *soundInput) {
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