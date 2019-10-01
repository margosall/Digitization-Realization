typedef struct {
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_reader;
    audio_element_handle_t filter;
    audio_element_handle_t raw_read;
    int16_t *buffer;
} sound_input_struct_t;

sound_input_struct_t *setupMic(int sampleRate);
void cleanUpMic(sound_input_struct_t *soundInput);
