#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define ADC_BITS 16      // range of sample value 2^ADC_BITS
#define PI 3.14159265358979323846
#define PERIOD 2.0 * PI // 2pi is 1 period in radians == 360 degrees

#if ADC_BITS == 16
#define SAMPLE_T uint16_t
#elif ADC_BITS == 8
#define SAMPLE_T uint8_t
#endif
#define SAMPLE_LENGTH 5
typedef struct Signal
{
    uint8_t bitCount;
    uint32_t sampleRate;
    double frequency;
    double amplitude;
    double phaseShift;
    char function[10];

    uint32_t sampleCount;
    SAMPLE_T *samples;
    SAMPLE_T converSionVal;
} SIGNAL_T;
void SignalInitialize(SIGNAL_T *signal, 
                      uint8_t bitCount,
                      uint32_t sampleRate,
                      double frequency,
                      double amplitude,
                      double phaseShift,
                      char function[10]
                      );
void SignalCalcSampleCount(SIGNAL_T *signal);
void SignalCalcConversionValue(SIGNAL_T *signal);

int8_t SignalGenerate(SIGNAL_T *sig);
int8_t SignalsAdd(SIGNAL_T a, SIGNAL_T b, SIGNAL_T *c);
int8_t SignalsSubtract(SIGNAL_T a, SIGNAL_T b, SIGNAL_T c);

void SignalPrint(SIGNAL_T sig);
void SignalFree(SIGNAL_T *sig);