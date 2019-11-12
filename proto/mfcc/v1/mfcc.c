#include "mfcc.h"

double frequencyToMel(double frequency)
{
    return 1125 * log(1 + frequency / 700);
}

double melToFrequency(double mel)
{
    return 700 * (exp(mel / 1125) - 1);
}

void uintToDouble(int16_t *samples, double *output, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        output[i] = (samples[i] / 32767.0);
    }
}

// https://batchloaf.wordpress.com/2013/12/07/simple-dft-in-c/
double dft(double *sample, double *P, uint32_t length)
{
    int n, k; // indices for time and frequency domains
    double *Xre = (double *)calloc(length, sizeof(double));
    double *Xim = (double *)calloc(length, sizeof(double));

    // Calculate DFT of x using brute force
    for (k = 0; k < length; ++k)
    {
        // Real part of X[k]
        Xre[k] = 0;
        for (n = 0; n < length; ++n)
        {
            Xre[k] += sample[n] * cos((n * k * PI2) / (length*1.0));
        }
        // Imaginary part of X[k]
        Xim[k] = 0;
        for (n = 0; n < length; ++n)
        {
            Xim[k] -= sample[n] * sin((n * k * PI2) / (length*1.0));
        }

        // Power at kth frequency bin
        P[k] = (pow(abs(Xre[k] * Xre[k] + Xim[k] * Xim[k]),2))/length;
    }
    free(Xre);
    free(Xim);
}