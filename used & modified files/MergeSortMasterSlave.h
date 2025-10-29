#ifndef MERGESORTMasterSlave_H
#define MERGESORTMasterSlave_H
#include "MergeSort.h"

typedef struct {
	int size;
	int model;
	int thread_num;
	int buffer_size;
}SettingsSort;

int dynamicForMergeSort(int* array, SettingsSort settings);
int taskingMergeSort(int* array, SettingsSort settings);
int integratedMasterMergeSort(int* array, SettingsSort settings);
void print_progress(int percent);
long long count_chunks(int size);

#endif