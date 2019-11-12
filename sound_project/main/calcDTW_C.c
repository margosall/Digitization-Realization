#include "calcDTW_C.h"

void printCostMatrix(int16_t **costMatrix, int32_t xSize, int32_t ySize) {
    for (int32_t col = 0; col < xSize; col++) {
        for (int32_t row = 0; row < ySize; row++) {
            printf("%hi\t", costMatrix[col][row]);
        }
        printf("\n");
    }
}

int32_t minimumOfVector(int32_t *vector, int32_t vectorLen) {
    int32_t minimum = vector[0];
    for (int32_t i = 1; i < vectorLen; i++) {
        if(minimum > vector[i]) {
            minimum = vector[i];
        }
    }
    return minimum;
}

int32_t maximumOfVector(int32_t *vector, int32_t vectorLen) {
    int32_t maximum = vector[0];
    for (int32_t i = 1; i < vectorLen; i++) {
        if(maximum < vector[i]) {
            maximum = vector[i];
        }
    }
    return maximum;
}

double calculateDTW(int16_t *signal1, int16_t *signal2, int32_t signalSize1, int32_t signalSize2) {
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


double calculateConstrainedDTW(int16_t *signal1, int16_t *signal2, int32_t signalSize1, int32_t signalSize2, int_fast32_t warpingConstant) {
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
