#ifndef MANDELBROTMasterSlave_H
#define MANDELBROTMasterSlave_H
#include "Mandelbrot.h"

typedef struct {
	double re_min;
	double re_max;
	double im_min;
	double im_max;
	int image_width;
	int image_height;
	int max_iterations;
	int block_size;
	int thread_num;
	int model; // 0 - dynamic, 1 - tasking, 2 - integrated
	int buffer_size; // size of the buffer for dynamic model
}SettingsMandelbrot;

void dynamicForMandelbrot(int** result_buffer, SettingsMandelbrot settings);
void taskingMandelbrot(int** result_buffer, SettingsMandelbrot settings);
void integratedMasterMandelbrot(int** result_buffer, SettingsMandelbrot settings);
void save_result_as_ppm(const char* filename, int** result_buffer);
void print_progress(int percent);

#endif