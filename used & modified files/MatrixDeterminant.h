#ifndef MATRIXDETERMINANT_H
#define MATRIXDETERMINANT_H

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

extern int BUFFERSIZEMATRIX;
extern int CHUNKCOUNTMATRIX;
extern int MATRIXSIZE;

typedef struct {
    int pivot_row_index;
    int target_row_index;
} t_input_matrix;

long generate_new_input_Matrix(t_input_matrix* input, double** matrix);
void process_Matrix(t_input_matrix* data, double**matrix);
long double determinant(double** matrix);

double** generateVandermondeMatrix(int size);
long double vandermondeDeterminant(int size);


#endif