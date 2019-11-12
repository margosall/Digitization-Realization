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
#define AUDIOCHUNKSIZE 512
#define SAMPLERATE 16000
#define DOWNSAMPLE_RATE 16000
#define OUTPUTCHANNELS 1


//INPUT SOURCES
#define MIC AUDIO_HAL_CODEC_MODE_ENCODE
#define LINEIN AUDIO_HAL_CODEC_MODE_LINE_IN
#define SOURCE MIC

void app_main()
{
    sound_input_struct_t *soundInput = setupRecording(SAMPLERATE, SOURCE, OUTPUTCHANNELS);
    printf("Free Size %u\r\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));
    xTaskCreatePinnedToCore(readSignal,
                "readSignal",
                8192*8,
                (void*)soundInput,
                1,
                NULL,
                0);
    printf("Free Size %u\r\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));

}

float int16ToFloatNormalization(int16_t toNormalize) {
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

void readSignal(void *soundInput) {
    sound_input_struct_t *soundSettings = (sound_input_struct_t *) soundInput;
    int bytesRead = 0;
    int counter;
    // int16_t testSignal[512] = {1307, 1016, -71, -901, -167, 280, -803, -1578, -2304, -3331, -4015, -3503, -2618, -333, 1520, 2681, 2376, 731, 555, 1344, 1576, 1878, 2280, 2635, 2910, 1750, -40, -990, -1439, -776, -123, -815, -1296, -1525, -1672, -2178, -2163, -1793, -1272, -426, 120, 64, 392, 992, 1188, 1215, 239, -240, -338, 427, 473, -273, -356, -465, 233, -257, -600, 588, 805, -245, -1078, -1099, -733, -918, -1000, -635, -1513, -1823, -1375, -228, 192, -1338, -1410, -462, -105, -237, 123, 892, 938, 1144, 901, 1237, 1896, 1823, 2546, 1871, 3, -140, 2208, 2602, 1244, 1307, 1612, 571, -454, -180, 104, 1212, 1114, 385, -497, -1069, -774, -701, -1, 698, 325, -266, -701, -932, -758, -1516, -1197, -284, 43, 1394, 3088, 2577, 1332, 395, -528, -793, -1003, -848, -940, 46, 260, -744, -689, -697, -1508, -1665, -2099, -1833, -272, 339, -14, -1315, -1660, -1305, 406, 2076, 2557, 2715, 1960, 1118, 1237, 1506, 246, -1108, -1342, -1621, -1821, -1094, -407, 312, 66, -862, -809, -489, -1171, -1015, -262, -584, -402, -147, 550, 722, 902, 397, -784, -452, 936, 2185, 1257, -575, -1113, -216, -293, -1888, -3200, -3631, -2708, -1403, -728, -630, -1521, -1402, 20, 285, -786, -1193, -449, 221, 662, 663, 852, 1291, 721, 76, -588, -793, 683, 2109, 3066, 2857, 2521, 2973, 3005, 1830, 983, 1912, 1119, -165, -244, 733, 1585, 482, -852, -79, 1435, 280, -421, -40, 300, 610, -438, -1758, -1109, 184, 375, 968, 640, 576, 1229, 259, -747, -774, -795, -783, 251, 1258, 1154, 342, -131, 344, 113, -1660, -3483, -2675, -516, 465, 91, -375, -175, -285, -727, -795, 478, 1728, 2114, 1725, 1313, 912, 142, -71, -1066, -1970, -2626, -2945, -2995, -1441, 901, 1306, 1903, 1703, 2036, 2046, 1151, 222, -955, -1592, -816, -664, -2444, -2635, -1375, 789, 1676, 2005, 2053, 1930, 2421, 2937, 2470, -754, -3899, -5155, -4969, -3803, -2675, -2767, -1930, -839, -1329, -841, -860, -1812, -1542, 447, 2000, 313, -1583, -2422, -1224, 1225, 1695, 1884, 1546, 584, 590, 764, 41, -348, 125, 583, 1246, 1589, 1968, 2585, 2745, 2547, 2708, 3708, 3124, 1281, 308, -498, -779, -388, -362, 89, 447, 142, 1082, 1683, 1160, 14, -1441, -1932, -1023, -900, -1251, -606, -486, -1006, -2305, -2182, -628, 1304, 1652, 1015, 1310, 1425, 1128, -111, -959, -1095, -1090, -1639, -2710, -1899, -114, -227, -273, 1421, 3172, 2502, 154, -1457, -495, 1089, 912, 623, 1182, 1337, 491, -190, -1033, -426, 69, -993, -1190, -206, 403, 865, 265, -1136, -697, -104, 1140, 1946, 1342, 334, 570, 1623, 2612, 3029, 267, -2137, -2831, -2162, -2008, -3777, -4424, -3034, -2712, -4418, -3429, -1925, -1435, -516, -574, -1254, -1168, -113, 532, 1429, 1094, 1297, 1483, 349, 498, 783, 166, 249, 1585, 3065, 4555, 3411, 1481, 470, 44, -303, -693, -974, -1401, -641, 634, 1543, 2091, 2203, 1722, 995, 612, 1567, 2455, 2351, 1367, 547, 497, 90, 310, 183, -927, -2238, -3503, -3040, -1732, -1060, -253, 617, 553, 1141, 1324, 243, -53, 81, 467, 1256, 2252, 2365, 1233, -1302, -2647, -2801, -3485, -3762, -3727, -2243, -324, 1763, 3135, 2890, 2218, 1694, 1971, 1137, -716, -1183, -1679, -2041, -1236, -371, -75, 161, 712, 1377, 1393, 443, 736, 743};
    // int32_t testSignalSize = sizeof(testSignal) / sizeof(testSignal[0]);
    // double distance = 0;
    unsigned int testSignalLen = AUDIOCHUNKSIZE;
    int aSampleRate = DOWNSAMPLE_RATE;
    csf_float aWinLen = 0.025; // 25ms
    csf_float aWinStep = 0.01; // 10ms per window movement
    int aNCep = 13; // Num of coefficents
    int aNFilters = 26; // Number of filters
    int aNFFT = 512; // FFT size
    int aLowFreq = 0; // Lowest frequency 
    int aHighFreq = 0; // If <= low, then it is samplerate / 2; Nyquist
    csf_float aPreemph = 0.97; 
    int aCepLifter = 22;
    int aAppendEnergy = 1; // Add spectral energy to aMFCC[0]
    csf_float* aWinFunc = calculateHamming(aSampleRate * aWinLen); // Windowing function should use hamming / hanning later TODO
    int frames = 0; // Frame counter
    int splitCounter = 0;

    for(;;) {
        csf_float **aMFCC = (csf_float **) malloc(sizeof(csf_float *));
        bytesRead = raw_stream_read(soundSettings->raw_read, 
                                   (char *)soundSettings->buffer,
                                   AUDIOCHUNKSIZE * sizeof(int16_t));
        frames = csf_mfcc(soundSettings->buffer,
                          testSignalLen, 
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

        for (int i = 0; i < ((frames * aNCep)); i++) {
            printf("%f ", *(*(aMFCC)+ i));
            if (splitCounter == 13) splitCounter = 0, printf("\n");
            splitCounter++;
        }

        free(aMFCC);
        // distance = calculateConstrainedDTW(testSignal, (int16_t *) soundSettings->buffer, testSignalSize, AUDIOCHUNKSIZE, 5);

        // printf("%f\n", distance);

        // for (counter = 0; counter < AUDIOCHUNKSIZE; counter++) {
        //     printf("%hi ", soundSettings->buffer[counter]);
        // }

        // printf("%d\n", bytesRead);
        // printf("\n");
    }
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