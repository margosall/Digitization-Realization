#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#define PI2 6.283185307179586

double frequencyToMel(double frequency);
double melToFrequency(double mel);
void uintToDouble(int16_t *samples, double *output, uint32_t length);
double dft(double *sample, double P[32768],uint32_t length);