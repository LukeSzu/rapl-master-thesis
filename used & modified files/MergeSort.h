#ifndef MERGESORT_H
#define MERGESORT_H

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

extern int BUFFERSIZE;
extern int arraySize;
extern int CHUNKCOUNT;

typedef struct {
	int* left; // pointer to the left part of the input
	int middle; // index of the middle element (first of the right part)
	int size; // size of the input to sort
    int force; //number of extra elements to consider in merge sort when the array size is not a power of two.
} t_input_sort;

long generate_new_input_sort(t_input_sort* input, int* array);
void process_sort(t_input_sort* data);

#endif