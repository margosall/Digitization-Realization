#ifndef CALCDTW_H
#define CALCDTW_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y)) 
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void printCostMatrix(int16_t **costMatrix, uint32_t xSize, uint32_t ySize);
void printCostMatrixFloat(float **costMatrix, uint32_t xSize, uint32_t ySize);
int32_t minimumOfVector(int32_t *vector, uint32_t vectorLen);
int32_t maximumOfVector(int32_t *vector, uint32_t vectorLen);
float minimumOfVectorFloat(float *vector, uint32_t vectorLen);
float maximumOfVectorFloat(float *vector, uint32_t vectorLen);
double calculateDTW(int16_t *signal1, int16_t *signal2, uint32_t signalSize1, uint32_t signalSize2);
double calculateConstrainedDTW(int16_t *signal1, int16_t *signal2, uint32_t signalSize1, uint32_t signalSize2, uint_fast32_t warpingConstant);
float calculateConstrainedDTWFloat(float *mfcc1, float *mfcc2, uint32_t mfcc1Len, uint32_t mfcc2Len, uint_fast32_t warpingConstant);
float calculateDistance(float *mfcc1, float *mfcc2, uint32_t mfcc1Len, uint32_t mfcc2Len, uint_fast32_t warpingConstant);

#endif