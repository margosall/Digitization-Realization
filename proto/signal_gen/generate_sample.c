// #include <stdio.h>
// #include <stdint.h>
// #include <math.h>
// #define PI 3.14159265358979323846

#include "signal.h"

// #define ADC_BITS 16      // range of sample value 2^ADC_BITS
// #define SAMPLE_RATE 10 // discreeting freq Hz
// #define SAMPLE_LENGTH 1  //s
// #define PERIOD 2.0 * PI
// #define FREQUENCY 1.0 //Hz
// #define N SAMPLE_RATE *SAMPLE_LENGTH
// #define AMPLITUDE 2 // V or whatever value f(t)

// #define HIGHEST_VALUE_CONVERSION ((1 << ADC_BITS) - 1)/(AMPLITUDE*2) // 2^ADC_BITS -1 / 2 because of +1 to all values

// #define PHASE_SHIFT 0.0

// #define FUNCTION sin

// #if ADC_BITS == 16
// #define SAMPLE_T uint16_t
// #elif ADC_BITS == 8
// #define SAMPLE_T uint8_t
// #endif

int main(void)
{
    SIGNAL_T sinusoidial={0};
    SIGNAL_T cosine={0};

    SIGNAL_T sum={0};

    SignalInitialize(&sinusoidial,16,1000,2,0.2,0,"sin");
    SignalInitialize(&cosine,16,1000,1,0.2,0,"cos");
    SignalGenerate(&sinusoidial);
    SignalGenerate(&cosine);

    SignalsAdd(sinusoidial, cosine, &sum);

    SignalPrint(sum);

    // SAMPLE_T audioSample[N] = {0};
    // for (int t = 0; t < N; t++)
    // {
    //     audioSample[t] = (SAMPLE_T)(
    //         (
    //         FUNCTION((((double)(t) * (1.0 / (double)N)) * (double)PERIOD * FREQUENCY) + PHASE_SHIFT) 
    //         * AMPLITUDE 
    //         + AMPLITUDE
    //         )
    //         *HIGHEST_VALUE_CONVERSION
    //         );
    // }
    // for (int t = 0; t < N; t++)
    // {
    //     printf("%d,%d\n", t,audioSample[t]);
    // }
    // // printf("\n");
    return 0;
}