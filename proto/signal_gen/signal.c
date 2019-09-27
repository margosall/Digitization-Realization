#include "signal.h"

void SignalInitialize(SIGNAL_T *signal, uint8_t bitCount, uint32_t sampleRate, double frequency, double amplitude, double phaseShift, char function[10])
{
    (*signal).bitCount = bitCount;
    (*signal).sampleRate = sampleRate;
    (*signal).frequency = frequency;
    (*signal).amplitude = amplitude;
    (*signal).phaseShift = phaseShift;
    strcpy((*signal).function, function);
    SignalCalcSampleCount(signal);
    SignalCalcConversionValue(signal);

    (*signal).samples = (SAMPLE_T *)calloc((*signal).sampleRate, sizeof(SAMPLE_T));
}

void SignalCalcSampleCount(SIGNAL_T *signal)
{
    (*signal).sampleCount = (*signal).sampleRate * 1;
}
void SignalCalcConversionValue(SIGNAL_T *signal)
{
    const SAMPLE_T highestValue = ((1 << ADC_BITS) - 1);
    (*signal).converSionVal = highestValue / ((*signal).amplitude * 2);
}

int8_t SignalGenerate(SIGNAL_T *signal)
{
    if (
        ((*signal).bitCount != 0) &&
        ((*signal).frequency != 0) &&
        ((*signal).amplitude != 0) &&
        (strlen((*signal).function) != 0) &&
        ((*signal).sampleCount != 0))
    {
        if (strcmp((*signal).function, "sin") == 0)
        {
            for (int t = 0; t < (*signal).sampleCount; t++)
            {
                (*signal).samples[t] = (SAMPLE_T)(
                    (
                        sin(
                            (((double)(t) * (1.0 / (double)(*signal).sampleCount)) *
                             (double)PERIOD * (*signal).frequency) +
                            (*signal).phaseShift) *
                            (*signal).amplitude +
                        (*signal).amplitude) *
                    (*signal).converSionVal);
            }
        }
        else if (strcmp((*signal).function, "cos") == 0)
        {
            for (int t = 0; t < (*signal).sampleCount; t++)
            {
                (*signal).samples[t] = (SAMPLE_T)(
                    (
                        cos(
                            (((double)(t) * (1.0 / (double)(*signal).sampleCount)) *
                             (double)PERIOD * (*signal).frequency) +
                            (*signal).phaseShift) *
                            (*signal).amplitude +
                        (*signal).amplitude) *
                    (*signal).converSionVal);
            }
        }
    }
    return -1;
}
int8_t SignalsAdd(SIGNAL_T a, SIGNAL_T b, SIGNAL_T *c)
{
    int i = 0;
    int32_t s1;
    int32_t s2;
    if ((a.sampleCount == b.sampleCount))
    {
        (*c).sampleCount = a.sampleCount;
        (*c).samples = (SAMPLE_T *)calloc((*c).sampleCount, sizeof(SAMPLE_T));
        for (; i < a.sampleCount; i++)
        {
            s1 = (a.samples[i]<a.converSionVal)? -a.samples[i] : a.samples[i];
            s2 = (b.samples[i]<a.converSionVal)? -b.samples[i] : b.samples[i];

            (*c).samples[i] = (uint16_t)(s1+s2);
        }
    }
}

void SignalPrint(SIGNAL_T sig)
{
    int t = 0;
    for (; t < sig.sampleCount; t++)
    {
        printf("%d,%d\n", t, sig.samples[t]);
    }
}

void SignalFree(SIGNAL_T *sig)
{
    if ((*sig).samples != NULL)
    {
        free((*sig).samples);
        (*sig).samples = NULL;
    }
}