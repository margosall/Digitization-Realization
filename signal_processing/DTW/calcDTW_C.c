#include "calcDTW_C.h"


void printCostMatrix(int16_t **costMatrix, uint32_t xSize, uint32_t ySize) {
    for (int32_t col = 0; col < xSize; col++) {
        for (int32_t row = 0; row < ySize; row++) {
            printf("%hi\t", costMatrix[col][row]);
        }
        printf("\n");
    }
}

void printCostMatrixFloat(float **costMatrix, uint32_t xSize, uint32_t ySize) {
    for (int32_t col = 0; col < xSize; col++) {
        for (int32_t row = 0; row < ySize; row++) {
            printf("%f\t", costMatrix[col][row]);
        }
        printf("\n");
    }
}

int32_t minimumOfVector(int32_t *vector, uint32_t vectorLen) {
    int32_t minimum = vector[0];
    for (int32_t i = 1; i < vectorLen; i++) {
        if(minimum > vector[i]) {
            minimum = vector[i];
        }
    }
    return minimum;
}

int32_t maximumOfVector(int32_t *vector, uint32_t vectorLen) {
    int32_t maximum = vector[0];
    for (int32_t i = 1; i < vectorLen; i++) {
        if(maximum < vector[i]) {
            maximum = vector[i];
        }
    }
    return maximum;
}

float minimumOfVectorFloat(float *vector, uint32_t vectorLen) {
    float minimum = vector[0];
    for (int32_t i = 1; i < vectorLen; i++) {
        if(minimum > vector[i]) {
            minimum = vector[i];
        }
    }
    return minimum;
}

float maximumOfVectorFloat(float *vector, uint32_t vectorLen) {
    float maximum = vector[0];
    for (int32_t i = 1; i < vectorLen; i++) {
        if(maximum < vector[i]) {
            maximum = vector[i];
        }
    }
    return maximum;
}


double calculateDTW(int16_t *signal1, int16_t *signal2, uint32_t signalSize1, uint32_t signalSize2) {
    double distance = 0;
    int32_t minVector[3] = {0};
    int32_t cost = 0;
    int32_t totalLoops = 0;
    uint16_t **costMatrix = (uint16_t**) calloc(1, sizeof(uint16_t *) * signalSize1);
    for (int32_t i = 0; i < signalSize1; i++) {
        costMatrix[i] = (uint16_t *) calloc(1, sizeof(uint16_t) * signalSize2);
    }

    costMatrix[0][0] = 0;

    for (int32_t i = 1; i < signalSize1; i++) {
        costMatrix[i][0] = INFINITY;
    }
    for (int32_t i = 1; i < signalSize2; i++) {
        costMatrix[0][i] = INFINITY;
    }

    for (int32_t i = 1; i < signalSize1; i++) {
        for (int32_t j = 1; j < signalSize2; j++) {
            cost = abs(signal1[i] - signal2[j]);
            minVector[0] = costMatrix[i - 1][j];
            minVector[1] = costMatrix[i ][j - 1];
            minVector[2] = costMatrix[i - 1][j - 1];
            costMatrix[i][j] = cost + minimumOfVector(minVector, 3);
        }
    }


    distance = sqrt(costMatrix[signalSize1 - 1][signalSize2 - 1]);
    for (int32_t i = 0; i < signalSize1; i++) {
        free(costMatrix[i]);
    }
    free(costMatrix);


    return distance;
    }


double calculateConstrainedDTW(int16_t *signal1, int16_t *signal2, uint32_t signalSize1, uint32_t signalSize2, uint_fast32_t warpingConstant) {
    double distance = 0;
    int32_t minVector[3] = {0};
    int32_t cost = 0;
    int32_t totalLoops = 0;
    int32_t w = 0;
    int32_t wCalc[2] = {0};
    int32_t rowMaxCalc[2] = {1, 0};
    int32_t rowMinCalc[2] = {signalSize2, 0};
    int32_t rowMax = 0;
    int32_t rowMin = 0;

    uint16_t **costMatrix = (uint16_t**) calloc(1, sizeof(uint16_t *) * signalSize1);
    for (int32_t i = 0; i < signalSize1; i++) {
        costMatrix[i] = (uint16_t *) calloc(1, sizeof(uint16_t) * signalSize2);
    }

    costMatrix[0][0] = 0;
    for (int32_t i = 1; i < signalSize1; i++) {
        costMatrix[i][0] = INFINITY;
    }
    for (int32_t i = 1; i < signalSize2; i++) {
        costMatrix[0][i] = INFINITY;
    }

    
    wCalc[0] = warpingConstant;
    wCalc[1] = abs(signalSize1 - signalSize2);
    w = maximumOfVector(wCalc, 2);

    for (int32_t i = 1; i < signalSize1; i++) {
        rowMaxCalc[1] = i - w;
        rowMinCalc[1] = i + w;
        rowMax = maximumOfVector(rowMaxCalc, 2);
        rowMin = minimumOfVector(rowMinCalc, 2);
        for (int32_t j = rowMax; j < rowMin; j++) {
            cost = abs(signal1[i] - signal2[j]);
            minVector[0] = costMatrix[i - 1][j];
            minVector[1] = costMatrix[i ][j - 1];
            minVector[2] = costMatrix[i - 1][j - 1];
            costMatrix[i][j] = cost + minimumOfVector(minVector, 3);
        }
    }

    distance = sqrt(costMatrix[signalSize1 - 1][signalSize2 - 1]);
    for (int32_t i = 0; i < signalSize1; i++) {
        free(costMatrix[i]);
    }
    free(costMatrix);


    return distance;
}

float calculateConstrainedDTWFloat(float *mfcc1, float *mfcc2, uint32_t mfcc1Len, uint32_t mfcc2Len, uint_fast32_t warpingConstant) {
    float distance = 0;
    float minVector[3] = {0};
    float cost = 0;
    uint32_t totalLoops = 0;
    uint32_t w = 0;
    int32_t wCalc[2] = {0};
    int32_t rowMaxCalc[2] = {1, 0};
    int32_t rowMinCalc[2] = {mfcc2Len, 0};
    int32_t rowMax = 0;
    int32_t rowMin = 0;

    float **costMatrix = (float**) malloc(sizeof(float *) * mfcc1Len);
    for (int32_t i = 0; i < mfcc1Len; i++) {
        costMatrix[i] = (float *) malloc(sizeof(float) * mfcc2Len);
    }

    for (int32_t i = 1; i < mfcc1Len; i++) {
        memset(costMatrix[i], 0, sizeof(float) * mfcc2Len);
    }

    for (int32_t i = 1; i < mfcc1Len; i++) {
        costMatrix[i][0] = INFINITY;
    }
    for (int32_t i = 1; i < mfcc2Len; i++) {
        costMatrix[0][i] = INFINITY;
    }

        
    wCalc[0] = warpingConstant;
    wCalc[1] = fabs(mfcc1Len - mfcc2Len);
    w = maximumOfVector(wCalc, 2);

    for (int32_t i = 1; i < mfcc1Len; i++) {
        rowMaxCalc[1] = i - w;
        rowMinCalc[1] = i + w;
        rowMax = maximumOfVector(rowMaxCalc, 2);
        rowMin = minimumOfVector(rowMinCalc, 2);
        for (int32_t j = rowMax; j < rowMin; j++) {
            cost = fabs((mfcc1[i] - mfcc2[j]) * (mfcc1[i] - mfcc2[j]));
            printf("%f\n", cost);
            minVector[0] = costMatrix[i - 1][j];
            minVector[1] = costMatrix[i ][j - 1];
            minVector[2] = costMatrix[i - 1][j - 1];
            costMatrix[i][j] = cost + minimumOfVectorFloat(minVector, 3);
        }
    }

    // printCostMatrixFloat(costMatrix, mfcc1Len, mfcc2Len);

    distance = sqrtf(costMatrix[mfcc1Len - 1][mfcc2Len - 1]);

    for (int32_t i = 0; i < mfcc1Len; i++) {
        free(costMatrix[i]);
    }
    free(costMatrix);


    return distance;
}


float calculateDistance(float *mfcc1, float *mfcc2, uint32_t mfcc1Len, uint32_t mfcc2Len, uint_fast32_t warpingConstant) {
    int32_t w = 0;
    int32_t jMax = 0;
    int32_t jMin = 0;
    int32_t i = 0, j = 0;
    float cost = 0;
    float minVector[3] = {0, 0, 0};


    float **costMatrix = (float**) malloc(sizeof(float *) * mfcc1Len);
    for (int32_t i = 0; i < mfcc1Len; i++) {
        costMatrix[i] = (float *) malloc(sizeof(float) * mfcc2Len);
    }

    for (i = 0; i < mfcc1Len; i++) {
        for (j = 0; j < mfcc2Len; j++) {
            costMatrix[i][j] = INFINITY;
        }
    }
    costMatrix[0][0] = 0;
    

    w = MAX(warpingConstant, abs(mfcc1Len - mfcc2Len));


    for (i = 1; i < mfcc1Len; i++) {
        jMax = MAX(1, i - w);
        jMin = MIN(mfcc2Len, i + w);
        for (j = jMax; j < jMin; j++) {
            costMatrix[i][j] = 0;
        }
    }

    // printCostMatrixFloat(costMatrix, mfcc1Len, mfcc2Len);

    for (i = 1; i < mfcc1Len; i++) {
        jMax = MAX(1, i - w);
        jMin = MIN(mfcc2Len, i + w);
        // printf("%d\t%d\n", jMax, jMin);
        for (j = jMax; j < jMin; j++) {
            cost = fabs(mfcc1[i] - mfcc2[j]);
            // printf("%d\t%d\t%f\t%f\t%f\n", i, j, mfcc1[i] - mfcc2[j], mfcc1[i], mfcc2[i]);
            minVector[0] = costMatrix[i-1][j];
            minVector[1] = costMatrix[i][j - 1];
            minVector[2] = costMatrix[i - 1][j - 1];
            costMatrix[i][j] = cost + minimumOfVectorFloat(minVector, 3);
        }
    }

    // printCostMatrixFloat(costMatrix, mfcc1Len, mfcc2Len);
    // printf("\n");

    for (int32_t i = 0; i < mfcc1Len; i++) {
        free(costMatrix[i]);
    }
    free(costMatrix);

    return costMatrix[mfcc1Len - 1][mfcc2Len - 1];
}