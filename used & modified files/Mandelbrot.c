#include "Mandelbrot.h"

int BUFFERSIZEMANDELBROT;
int CHUNKCOUNTMANDELBROT;
double re_min, re_max, im_min, im_max;
int image_width, image_height;
int max_iterations, block_size;

long generate_new_input_Mandelbrot(t_input_mandelbrot* input) { // returned value: how many items generated

	static int howmanygenerated = 0;
	static int x = 0, y = 0;

	//Sets the maximum number of chunks to generate this time
	int max = BUFFERSIZEMANDELBROT;
	if (BUFFERSIZEMANDELBROT > (CHUNKCOUNTMANDELBROT - howmanygenerated))
		max = CHUNKCOUNTMANDELBROT - howmanygenerated;

	//Divides image into smaller pieces block_size x block_size
	int counter = 0;
	for (counter = 0;counter < max;counter++) {
		input[counter].block_x = x;
		input[counter].block_y = y++;

		howmanygenerated++;
		if (y == image_height/block_size) {
			x++;
			y = 0;
			if (x == image_width/block_size) {
				break;
			}
		}
	}

	return counter;
}

void process_Mandelbrot(t_input_mandelbrot* packet, int** result_buffer) {

	//Standard Mandelbrot set calculation
	for (int dy = 0; dy < block_size; ++dy) {
		for (int dx = 0; dx < block_size; ++dx) {
			int px = packet->block_x * block_size + dx;
			int py = packet->block_y * block_size + dy;

			if (px >= image_width || py >= image_height)
				continue;

			double re = re_min + px * (re_max - re_min) / image_width;
			double im = im_min + py * (im_max - im_min) / image_height;

			double x = 0.0, y = 0.0;
			int iter = 0;
			while (x * x + y * y <= 4.0 && iter < max_iterations) {
				double x_new = x * x - y * y + re;
				y = 2 * x * y + im;
				x = x_new;
				++iter;
			}

			result_buffer[py][px] = iter;
		}
	}
}
