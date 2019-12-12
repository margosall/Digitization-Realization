#ifndef DTWDIST_H
#define DTWDIST_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y)) 
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void printCostMatrixFloat(float **costMatrix, uint32_t xSize, uint32_t ySize);

float minimumOfVectorFloat(float *vector, uint32_t vectorLen);
float maximumOfVectorFloat(float *vector, uint32_t vectorLen);

float calculateDistance(float *mfcc1, float *mfcc2, uint32_t mfcc1Len, uint32_t mfcc2Len, uint_fast32_t warpingConstant);
float calculateDistanceQuitEarly(float *mfcc1, float *mfcc2, uint32_t mfcc1Len, uint32_t mfcc2Len, uint_fast32_t warpingConstant, float bestSoFar);

float LBKeogh(float *mfcc1, float *mfcc2, uint32_t mfccLen, int_fast32_t warpingConstant, uint32_t bestSoFar);
#endif