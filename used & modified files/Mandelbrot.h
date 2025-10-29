#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

extern int BUFFERSIZEMANDELBROT; //Size of buffer for dynamic model
extern int CHUNKCOUNTMANDELBROT; //Number of chunks to process
extern double re_min, re_max, im_min, im_max; 
extern int image_width, image_height;
extern int max_iterations, block_size;

typedef struct {
    int block_x;
    int block_y;
} t_input_mandelbrot;

long generate_new_input_Mandelbrot(t_input_mandelbrot* input);
void process_Mandelbrot(t_input_mandelbrot* data, int** result_buffer);

#endif