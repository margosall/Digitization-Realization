#ifndef CALCDTW_H
#define CALCDTW_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void printCostMatrix(int16_t **costMatrix, int32_t xSize, int32_t ySize);
int32_t minimumOfVector(int32_t *vector, int32_t vectorLen);
int32_t maximumOfVector(int32_t *vector, int32_t vectorLen);
double calculateDTW(int16_t *signal1, int16_t *signal2, int32_t signalSize1, int32_t signalSize2);
double calculateConstrainedDTW(int16_t *signal1, int16_t *signal2, int32_t signalSize1, int32_t signalSize2, int_fast32_t warpingConstant);

#endif