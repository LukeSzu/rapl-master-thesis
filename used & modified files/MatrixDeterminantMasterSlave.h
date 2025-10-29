#ifndef MATRIXDETERMINANTMasterSlave_H
#define MATRIXDETERMINANTMasterSlave_H
#include "MatrixDeterminant.h"

typedef struct {
	int size;
	int vandermonde;
	int model;
	int prefab;
	int thread_num;
	int buffer_size;
}SettingsMatrix;

int dynamicForMatrixDeterminant(double** matrix, SettingsMatrix settings);
int taskingMatrixDeterminant(double** matrix, SettingsMatrix settings);
int integratedMasterMatrixDeterminant(double** matrix, SettingsMatrix settings);
void print_progress(int percent);

#endif