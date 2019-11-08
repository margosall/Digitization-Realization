#include <stdio.h>


void matrixMult(const double matrix1[4], const double matrix2[4], double
                outputArg1[4])
{
  int i0;
  double d0;
  for (i0 = 0; i0 < 2; i0++) {
    outputArg1[i0] = 0.0;
    d0 = matrix2[i0 + 2];
    outputArg1[i0] = matrix2[i0] * matrix1[0] + d0 * matrix1[1];
    outputArg1[i0 + 2] = 0.0;
    outputArg1[i0 + 2] = matrix2[i0] * matrix1[2] + d0 * matrix1[3];
  }
}



int main(void) {
    double outputArg1[4] = {0};
    const double in1[4] = {1, 2, 3, 4};
    const double in2[4] = {1, 2, 3, 6};


    matrixMult(in1, in2, outputArg1);

    for (int i = 0; i < 4 ; i++) {
        printf("%f ", outputArg1[i]);
        if (i == 1) {
          printf("\n");
        }
    }
}