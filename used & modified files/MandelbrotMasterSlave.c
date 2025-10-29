/*
Copyright 2017, Paweł Czarnul pawelczarnul@pawelczarnul.com

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*

Implementation Note: The provided source code has been optimized for performance. 
Key modifications from a textbook approach include:

Removal of worker return values: All threads operate on a shared address space, 
    writing results directly to their final memory locations.

Elimination of the master merge phase: As a consequence of point 1, no final merge 
    step by the master thread is required, as the shared data is already in its complete, 
    final state after the workers finish.

Original implementations are included in this repository.

*/
#include "MandelbrotMasterSlave.h"

void dynamicForMandelbrot(int** result_buffer, SettingsMandelbrot settings) {

    int threadnum = settings.thread_num;
	re_min = settings.re_min;
	re_max = settings.re_max;
	im_min = settings.im_min;
	im_max = settings.im_max;
	image_width = settings.image_width;
	image_height = settings.image_height;
	max_iterations = settings.max_iterations;
	block_size = settings.block_size;
    BUFFERSIZEMANDELBROT = settings.buffer_size;

	CHUNKCOUNTMANDELBROT = image_height * image_width / (block_size * block_size);

    //t_input_mandelbrot input[BUFFERSIZEMANDELBROT]; // firstly some input data is generated, then the master adds some data
    t_input_mandelbrot* input = (t_input_mandelbrot*)malloc(sizeof(t_input_mandelbrot) * BUFFERSIZEMANDELBROT);
    if (input == NULL) {
        perror("Memory allocation failed (array)");
        exit(EXIT_FAILURE);
    }
    long myinputindex;
    long lastgeneratedcount; // how many items were generated last time
    int work = 1;

    #pragma omp parallel private(myinputindex) shared(work,input,lastgeneratedcount) num_threads(threadnum)
    {
        long processedcount = 0;
        int processdata = 1;

        do {
            #pragma omp master
            {
                lastgeneratedcount = generate_new_input_Mandelbrot(input);
                processedcount += lastgeneratedcount;
                #ifdef _DEBUG
                    print_progress((int)(100 * processedcount / CHUNKCOUNTMANDELBROT));
                #endif
                // master checks if there is more data to process
                if (processedcount >= CHUNKCOUNTMANDELBROT || lastgeneratedcount == 0) {
                    processdata = 0;
                    #pragma omp atomic write
                    work = 0; // make slaves finish
                }
            }

            #pragma omp barrier
            #pragma omp atomic read
            processdata = work;

            #pragma omp for schedule(dynamic,1)
            for (myinputindex = 0;myinputindex < lastgeneratedcount;myinputindex++)
            {
                process_Mandelbrot(&(input[myinputindex]), result_buffer);
            }
        } while (processdata);
    }
    free(input);
}

void taskingMandelbrot(int** result_buffer, SettingsMandelbrot settings) {

    int threadnum = settings.thread_num;
    re_min = settings.re_min;
    re_max = settings.re_max;
    im_min = settings.im_min;
    im_max = settings.im_max;
    image_width = settings.image_width;
    image_height = settings.image_height;
    max_iterations = settings.max_iterations;
    block_size = settings.block_size;
	BUFFERSIZEMANDELBROT = settings.buffer_size;


    CHUNKCOUNTMANDELBROT = image_height * image_width / (block_size * block_size);

    //t_input_mandelbrot input[BUFFERSIZEMANDELBROT]; // firstly some input data is generated, then the master adds some data
	t_input_mandelbrot* input = (t_input_mandelbrot*)malloc(sizeof(t_input_mandelbrot) * BUFFERSIZEMANDELBROT);
    if (input == NULL) {
        perror("Memory allocation failed (array)");
        exit(EXIT_FAILURE);
    }
    long myinputindex;
    long lastgeneratedcount; // how many items were generated last time


#pragma omp parallel private(myinputindex) shared(input,lastgeneratedcount) num_threads(threadnum)
    {

#pragma omp single
        {
            long processedcount = 0;

            do {
                lastgeneratedcount = generate_new_input_Mandelbrot(input);
                // now create tasks that will deal with data packets
                for (myinputindex = 0;myinputindex < lastgeneratedcount;myinputindex++)
                {
#pragma omp task firstprivate(myinputindex) shared(input)
                    {
                        // now each task is processed independently and can store its result into an appropriate buffer
                        process_Mandelbrot(&(input[myinputindex]), result_buffer);
                    }
                }
                // wait for tasks 
#pragma omp taskwait   
                processedcount += lastgeneratedcount;
                #ifdef _DEBUG
                    print_progress((int)(100 * processedcount / CHUNKCOUNTMANDELBROT));
                #endif
            } while (processedcount < CHUNKCOUNTMANDELBROT && lastgeneratedcount != 0);
        }
    }
    free(input);
}

void integratedMasterMandelbrot(int** result_buffer, SettingsMandelbrot settings) {

    int threadnum = settings.thread_num;
    re_min = settings.re_min;
    re_max = settings.re_max;
    im_min = settings.im_min;
    im_max = settings.im_max;
    image_width = settings.image_width;
    image_height = settings.image_height;
    max_iterations = settings.max_iterations;
    block_size = settings.block_size;
    BUFFERSIZEMANDELBROT = settings.buffer_size;

    CHUNKCOUNTMANDELBROT = image_height * image_width / (block_size * block_size);


    //t_input_mandelbrot input[BUFFERSIZEMANDELBROT]; // firstly some input data is generated, then the master adds some data
    t_input_mandelbrot* input = (t_input_mandelbrot*)malloc(sizeof(t_input_mandelbrot) * BUFFERSIZEMANDELBROT);
    if (input == NULL) {
        perror("Memory allocation failed (array)");
        exit(EXIT_FAILURE);
    }

    long currentinputindex = 0; // points to the next item to fetch

    long lastgeneratedcount; // how many items were generated last time
    omp_lock_t inputoutputlock;

    long processedcount = 0; // should finally reach CHUNKCOUNT
    omp_init_lock(&inputoutputlock);

    // firstly generate BUFFERSIZE data chunks of input data
    lastgeneratedcount = generate_new_input_Mandelbrot(input);
    int active_workers = 0;
#pragma omp parallel shared(input,currentinputindex,lastgeneratedcount,processedcount) num_threads(threadnum)
    {
        // each thread acts as a slave

        int processdata;
        long myinputindex;
        int finish;
        do {
            processdata = 0;
            finish = 0;
            omp_set_lock(&inputoutputlock);
            if (processedcount < CHUNKCOUNTMANDELBROT && lastgeneratedcount != 0) {
                myinputindex = currentinputindex;
                if (currentinputindex < lastgeneratedcount) {
                    currentinputindex++;
                    processdata = 1;
                    #pragma omp atomic update
                        active_workers++;
                }
            }
            else {
                finish = 1;
            }
            omp_unset_lock(&inputoutputlock);

            if (processdata) {
                // now process the input data chunkprocess(&(input[myinputindex]));
                process_Mandelbrot(&(input[myinputindex]), result_buffer);
                #pragma omp atomic update
                    active_workers--;

                omp_set_lock(&inputoutputlock);
                if (currentinputindex == lastgeneratedcount && active_workers == 0) {
                    processedcount += lastgeneratedcount;
                #ifdef _DEBUG
					print_progress((int)(100 * processedcount / CHUNKCOUNTMANDELBROT));
                #endif
                    if (processedcount < CHUNKCOUNTMANDELBROT) {
                        lastgeneratedcount = generate_new_input_Mandelbrot(input);
                        currentinputindex = 0;
                    }
                }
                omp_unset_lock(&inputoutputlock);
            }
        } while (!finish);
    }

    omp_destroy_lock(&inputoutputlock);
    free(input);
}

void save_result_as_ppm(const char* filename, int** result_buffer) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return;
    }

    // Znajd� maksymaln� warto�� iteracji
    int max_val = 0;
    for (int y = 0; y < image_height; ++y)
        for (int x = 0; x < image_width; ++x)
            if (result_buffer[y][x] > max_val)
                max_val = result_buffer[y][x];
    if (max_val == 0) max_val = 1;

    fprintf(fp, "P6\n%d %d\n255\n", image_width, image_height);


    for (int y = 0; y < image_height; ++y) {
        for (int x = 0; x < image_width; ++x) {
            int iter = result_buffer[y][x];
            //double t = (double)iter / max_val;
            int val = log(iter + 1) / log(max_iterations + 1) * 255;

            // Proste kolorowanie (t�cza)
            unsigned char r = (unsigned char)(val % 256);
            unsigned char g = (unsigned char)((val * 5) % 256);
            unsigned char b = (unsigned char)((val * 13) % 256);

            fwrite(&r, 1, 1, fp);
            fwrite(&g, 1, 1, fp);
            fwrite(&b, 1, 1, fp);
        }
    }

    fclose(fp);
}

void print_progress(int percent) {
    printf("\rProgress: [%-50s] %3d%%",
        "##################################################" + (50 - percent / 2),
        percent);
    fflush(stdout);
}