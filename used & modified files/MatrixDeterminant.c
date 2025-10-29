#include "MatrixDeterminant.h"

int BUFFERSIZEMATRIX;
int CHUNKCOUNTMATRIX;
int MATRIXSIZE;

long generate_new_input_Matrix(t_input_matrix* input, double** matrix) { // returned value: how many items generated

    static int howmanygenerated = 0;
    static int x = 0, y = 1;

    int max = BUFFERSIZEMATRIX;
    if (BUFFERSIZEMATRIX > (CHUNKCOUNTMATRIX - howmanygenerated))
        max = CHUNKCOUNTMATRIX - howmanygenerated;

	//Generate which matrix[x][y] to process
    //Gaussian elimination
    int counter = 0;
    for (counter = 0;counter < max;counter++) {
		input[counter].pivot_row_index = x;
		input[counter].target_row_index = y++;
		howmanygenerated++;
        if (y == MATRIXSIZE) {
            x++;
			y = x + 1;
			if (x == MATRIXSIZE) {
				break;
			}
        }
    }
    return counter;
}

void process_Matrix(t_input_matrix* data, double** matrix) {

	int pivot_row_index = data->pivot_row_index;
	int target_row_index = data->target_row_index;
	
	double factor = matrix[target_row_index][pivot_row_index] / matrix[pivot_row_index][pivot_row_index];

	for (int i = 0; i < MATRIXSIZE; i++) {
		matrix[target_row_index][i] -= factor * matrix[pivot_row_index][i];
	}
	return;
}

long double determinant(double** matrix) {
    long double det = 1.0;
	for (int i = 0; i < MATRIXSIZE; i++) {
		det *= matrix[i][i];
	}
	return det;
}

double** generateVandermondeMatrix(int size) {
    double** matrix = (double**)malloc(size * sizeof(double*));
    if (matrix == NULL) {
        perror("Memory allocation failed (rows)");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        matrix[i] = (double*)malloc(size * sizeof(double));
        if (matrix[i] == NULL) {
            perror("Memory allocation failed (columns)");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < size; i++) {
        double base = (double)(i + 1);  // x_i
        for (int j = 0; j < size; j++) {
            matrix[i][j] = pow(base, j);  // x_i^j
        }
    }

    return matrix;
}

long double vandermondeDeterminant(int size) {
	double* x = (double*)malloc(size * sizeof(double));
    if (x == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
	for (int i = 1; i <= size; i++) {
		x[i - 1] = (double)i;
	}
    long double det = 1.0;
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            det *= (x[j] - x[i]);
        }
    }
    return det;
}