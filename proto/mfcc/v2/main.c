#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "c_speech_features.h"
csf_float * calculateHamming(int windowLength) {
    csf_float *hammWindow = (csf_float *) malloc(sizeof(csf_float) * windowLength);
        for (int32_t i = 0; i < windowLength; i++) {
            hammWindow[i] = 0.54 - 0.46 * cosf((2 * M_PI * i)/ (windowLength - 1));
        }
    return hammWindow;
}


#define CHUNK_SIZE 32768
#define SAMPLE_RATE 16000

void ReadData(int16_t sample[CHUNK_SIZE])
{
    int i = 0;
    FILE *sampleFile;
    sampleFile = fopen("sample.csv", "r");
    int16_t dump=0;

    for(int i =0;i<CHUNK_SIZE;i++)
    {
        if(fscanf(sampleFile, "%hi, ", &(sample[i])) == EOF)
        {
            exit(0);
        } // %lf tells fscanf to read a double
    }
    fclose(sampleFile);
}

int main(void)
{
    unsigned int testSignalLen = CHUNK_SIZE;
    int aSampleRate = 16000;
    csf_float aWinLen = 0.005; // 25ms
    csf_float aWinStep = 0.001; // 10ms per window movement
    int aNCep = 13; // Num of coefficents
    int aNFilters = 26; // Number of filters
    int aNFFT = 512; // FFT size
    int aLowFreq = 0; // Lowest frequency 
    int aHighFreq = 8000; // If <= low, then it is samplerate / 2; Nyquist
    csf_float aPreemph = 0.97; 
    int aCepLifter = 22;
    int aAppendEnergy = 1; // Add spectral energy to aMFCC[0]
    int frames = 0; // Frame counter
    int splitCounter = 0;

    csf_float* aWinFunc = calculateHamming(aSampleRate * aWinLen); // Windowing function should use hamming / hanning later TODO
    int16_t buffer[CHUNK_SIZE]={0};
    ReadData(buffer);
    csf_float **aMFCC = (csf_float **) malloc(sizeof(csf_float *));
    frames = csf_mfcc(buffer,
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

        printf("[");
        for (int i = 0; i < ((frames * aNCep)); i++) {
            printf("%f", *(*(aMFCC)+ i));
            if (splitCounter == 13) {splitCounter = 0, printf(";\n");}
            else
            {
                if(i < (frames*aNCep)-1)
                {
                    printf(",");
                }
            }
            
            splitCounter++;
        }
        printf("]");

    free(aMFCC);
    printf("\n");
    return 0;
}