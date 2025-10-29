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
#include "MatrixDeterminantMasterSlave.h"

int dynamicForMatrixDeterminant(double** matrix, SettingsMatrix settings) {

	int size = settings.size;
    int threadnum = settings.thread_num; // should be at least 2 - master and 1+ slave(s)
	MATRIXSIZE = size;
    CHUNKCOUNTMATRIX = (size - 1) * size / 2;
	BUFFERSIZEMATRIX = settings.buffer_size;

	t_input_matrix* input = (t_input_matrix*)malloc(sizeof(t_input_matrix) * BUFFERSIZEMATRIX);
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
                lastgeneratedcount = generate_new_input_Matrix(input, matrix);
                processedcount += lastgeneratedcount;
                #ifdef _DEBUG
                    print_progress((int)(100 * processedcount / CHUNKCOUNTMATRIX));
                #endif
                // master checks if there is more data to process
                if (processedcount >= CHUNKCOUNTMATRIX || lastgeneratedcount == 0) {
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
                process_Matrix(&(input[myinputindex]), matrix);
            }
        } while (processdata);
    }
    free(input);
    return 0;
}

int taskingMatrixDeterminant(double** matrix, SettingsMatrix settings) {

    int size = settings.size;
    int threadnum = settings.thread_num; // should be at least 2 - master and 1+ slave(s)
    MATRIXSIZE = size;
    CHUNKCOUNTMATRIX = (size - 1) * size / 2;
    BUFFERSIZEMATRIX = settings.buffer_size;

    t_input_matrix* input = (t_input_matrix*)malloc(sizeof(t_input_matrix) * BUFFERSIZEMATRIX);
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
                lastgeneratedcount = generate_new_input_Matrix(input, matrix);
                
                // now create tasks that will deal with data packets
                for (myinputindex = 0;myinputindex < lastgeneratedcount;myinputindex++) //MYINPUTINDEX
                {
                    // now each task is processed independently
                    #pragma omp task firstprivate(myinputindex) shared(input)
                    {
                        process_Matrix(&(input[myinputindex]), matrix);
                    }
                }
                // wait for tasks 
                #pragma omp taskwait   
                processedcount += lastgeneratedcount;
                #ifdef _DEBUG
                    print_progress((int)(100 * processedcount / CHUNKCOUNTMATRIX));
                #endif
            } while (processedcount < CHUNKCOUNTMATRIX && lastgeneratedcount != 0);
        }
    }
    free(input);
    return 0;
}

int integratedMasterMatrixDeterminant(double** matrix, SettingsMatrix settings) {

	int size = settings.size;
    int threadnum = settings.thread_num; // should be at least 2 - master and 1+ slave(s)
    MATRIXSIZE = size;
    CHUNKCOUNTMATRIX = (size - 1) * size / 2;
    BUFFERSIZEMATRIX = settings.buffer_size;

    t_input_matrix* input = (t_input_matrix*)malloc(sizeof(t_input_matrix) * BUFFERSIZEMATRIX);
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
    lastgeneratedcount = generate_new_input_Matrix(input, matrix);
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
            if (processedcount < CHUNKCOUNTMATRIX && lastgeneratedcount != 0) {
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
                #pragma omp atomic update
                    active_workers--;
                
                omp_set_lock(&inputoutputlock);
                if (currentinputindex == lastgeneratedcount && active_workers == 0) {
                    processedcount += lastgeneratedcount;
                    #ifdef _DEBUG
                        print_progress((int)(100 * processedcount / CHUNKCOUNTMATRIX));
                    #endif
                    if (processedcount < CHUNKCOUNTMATRIX) {
                        lastgeneratedcount = generate_new_input_Matrix(input, matrix);
                        currentinputindex = 0;
                    }
                }
                omp_unset_lock(&inputoutputlock);
            }
        } while (!finish);
    }
    free(input);
    omp_destroy_lock(&inputoutputlock);
    return 0;
}
