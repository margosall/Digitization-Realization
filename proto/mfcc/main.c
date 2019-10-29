#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mfcc.h"
#define SAMPLES_COUNT 32768

#define SAMPLE_RATE 16000
#define FREQ_MIN 0
#define FREQ_MAX SAMPLE_RATE / 2
#define SAMPLE_RATE_KHZ (SAMPLE_RATE / 1000)
#define DISCRETE_PERIOD 1.0 / (SAMPLE_RATE*1.0)

#define WINDOW_LENGTH_MS 25
#define WINDOW_LENGTH_N (WINDOW_LENGTH_MS *SAMPLE_RATE_KHZ)

#define FRAME_COUNT (SAMPLES_COUNT/WINDOW_LENGTH_N)
#define DURATION ((SAMPLES_COUNT*1.0) /(SAMPLE_RATE*1.0)) 
void ReadData(uint16_t sample[SAMPLES_COUNT])
{
    int i = 0;
    FILE *sampleFile;
    sampleFile = fopen("sample.csv", "r");

    while (fscanf(sampleFile, "%hi, ", &(sample[i])) != EOF) // %lf tells fscanf to read a double
    {
        i++;
    }
    fclose(sampleFile);
}
int main(void)
{
    int16_t sampleRaw[SAMPLES_COUNT] = {0};
    double sample[SAMPLES_COUNT] = {0};
    double **P=(double**)calloc(FRAME_COUNT,sizeof(double **));
    for(int i = 0;i<FRAME_COUNT;i++)
    {
        P[i] = (double *)calloc(WINDOW_LENGTH_N,sizeof(double));
    }
    uint16_t i = 0;
    ReadData(sampleRaw);
    printf("AUDIO DURATION: %lf\n",DURATION);
    // for(i=0;i<10;i++)
    // {
    //     printf("[%d] %hi\n",i,sample[i]);

    // }
    uintToDouble(sampleRaw,sample,SAMPLES_COUNT);

    for(uint16_t frame=0;frame<FRAME_COUNT;frame++)
    {
        // printf("Frame: %d\n",frame*WINDOW_LENGTH_N);
        dft(&(sample[frame*WINDOW_LENGTH_N]),P[frame],WINDOW_LENGTH_N);
    }

    for(int i = 0;i<FRAME_COUNT;i++)
    {
        free(*(P+i));
    }
    free(P);
    return 0;
}