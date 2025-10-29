#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <string.h>
#include "MandelbrotMasterSlave.h"
#include "MergeSortMasterSlave.h"
#include "MatrixDeterminantMasterSlave.h"


//To run the program correctly there is only needed to add the first argument
// - Mb for Mandelbrot
// - Mx for Matrix Determinant
// - St for Merge Sort

//For getting help with arguments add -help or -h as the second argument
//Example: ./mgr Mb -help
//All arguments are optional, but the first one is required
//All arguments have their default values, so if you want to run the program with default values, just add the first argument



int* g_array = NULL;
//Mandelbrot
void runMandelbrot(int argc, char** argv);
void displayMandelbrotSettings(SettingsMandelbrot settings);
void displayMandelbrotHelp();

//Matrix
void runMatrixDeterminant(int argc, char** argv);
void displayMatrixSettings(SettingsMatrix settings);
void displayMatrixHelp();
double** generateMatrix(int size);

// Merge Sort
void runMergeSort(int argc, char** argv);
void displayMergeSortSettings(SettingsSort settings);
void displayMergeSortHelp();
int* generateArray(int size);

int main(int argc, char** argv) {

#ifdef _DEBUG
	FILE* fp = fopen("debug.txt", "w");
	fclose(fp);
#endif
   
	if (argc > 1) {
		if (!strcmp(argv[1], "Mb")) {
			if (argc > 2 && (!strcmp(argv[2], "-help") || !strcmp(argv[2], "-h"))) {
				// Display help for Mandelbrot arguments
				displayMandelbrotHelp();
			}
			else {
				//Run Mandelbrot calculation with remaining arguments
				runMandelbrot(argc - 2, argv + 2);
			}
		}
		else if (!strcmp(argv[1], "Mx")) {
			if (argc > 2 && (!strcmp(argv[2], "-help") || !strcmp(argv[2], "-h"))) {
				// Display help for Matrix arguments
				displayMatrixHelp();
			}
			else {
				//Run Matrix Determinant calculation with remaining arguments
				runMatrixDeterminant(argc - 2, argv + 2);
			}
		}
		else if (!strcmp(argv[1], "St")) {
			if (argc > 2 && (!strcmp(argv[2], "-help") || !strcmp(argv[2], "-h"))) {
				//Display help for Merge Sort arguments
				displayMergeSortHelp();
			}
			else {
				//Run Merge Sort calculation with remaining arguments
				runMergeSort(argc - 2, argv + 2);
			}
		}
		else {
			printf("Invalid first argument. Use 'Mb' for Mandelbrot, 'Mx' for Matrix Determinant, or 'St' for Merge Sort.\n");
		}
	}
	else {
		printf("No arguments provided. Use 'Mb' for Mandelbrot, 'Mx' for Matrix Determinant, or 'St' for Merge Sort.\n");
	}
		
    return 0;
}
//Mandelbrot
void runMandelbrot(int argc, char** argv) {
	SettingsMandelbrot settings;
	settings.re_min = -2.0;
	settings.re_max = 1.0;
	settings.im_min = -1.5;
	settings.im_max = 1.5;
	settings.max_iterations = 1000;
	settings.image_width = 1920;
	settings.image_height = 1080;
	settings.block_size = 8;
	settings.thread_num = 4;
	settings.buffer_size = 512;
	settings.model = 0; // 0 - dynamic, 1 - tasking, 2 - integrated

	// Parse command line arguments
	for (int i = 0; i < argc; i+=2) {
		if (!strcmp(argv[i], "-rmin")) {
			settings.re_min = atof(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-rmax")) {
			settings.re_max = atof(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-imin")) {
			settings.im_min = atof(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-imax")) {
			settings.im_max = atof(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-w")) {
			settings.image_width = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-h")) {
			settings.image_height = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-b")) {
			settings.block_size = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-t")) {
			settings.thread_num = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-it")) {
			settings.max_iterations = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-model")) {
			settings.model = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-bs")) {
			settings.buffer_size = atoi(argv[i + 1]);
		}
		else {
			printf("Invalid argument: %s\n", argv[i]);
			exit(0);
		}
	}
	// Display the settings (once)
	#ifdef _DEBUG
		displayMandelbrotSettings(settings);
	#endif
	// Allocate memory for the result buffer
		int** result_buffer = malloc(settings.image_height * sizeof(int*));
	if (result_buffer == NULL) {
		perror("Memory allocation failed (image_height)");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < settings.image_height; ++i) {
		result_buffer[i] = calloc(settings.image_width, sizeof(int));
		if (result_buffer[i] == NULL) {
			perror("Memory allocation failed (image_width)");
			exit(EXIT_FAILURE);
		}
	}
	// Run Mandelbrot calculation based on the selected model
	switch (settings.model) {
		case 0: dynamicForMandelbrot(result_buffer, settings); break;
		case 1: taskingMandelbrot(result_buffer, settings); break;
		case 2: integratedMasterMandelbrot(result_buffer, settings); break;
		default: printf("Bad model\n"); break;
	}
	// Save the result as a PPM file
	#ifdef _DEBUG
		//save_result_as_ppm("mandelbrot.ppm", result_buffer);
	#endif
	// Free the result buffer
	for (int i = 0; i < settings.image_height; i++) {
		free(result_buffer[i]);
	}
	free(result_buffer);
	#ifdef _DEBUG
		print_progress(100);
		printf("\nMandelbrot set calculation completed. Result saved to mandelbrot.ppm\n");
	#endif
}
void displayMandelbrotSettings(SettingsMandelbrot settings) {
	const char* model_name;

	switch (settings.model) {
	case 0: model_name = "dynamic"; break;
	case 1: model_name = "tasking"; break;
	case 2: model_name = "integrated"; break;
	default: model_name = "unknown"; break;
	}

	printf("----- Settings -----\n");
	printf("Resolution      : %d x %d\n", settings.image_width, settings.image_height);
	printf("Complex range   : re = [%.2f, %.2f], im = [%.2f, %.2f]\n",
		settings.re_min, settings.re_max, settings.im_min, settings.im_max);
	printf("Max iterations  : %d\n", settings.max_iterations);
	printf("Block size      : %d\n", settings.block_size);
	printf("Thread count    : %d\n", settings.thread_num);
	printf("Buffer size     : %d\n", settings.buffer_size);
	printf("Parallel model  : %s\n", model_name);
	printf("--------------------\n");
}
void displayMandelbrotHelp() {
	printf("Usage: Mandelbrot [options]\n");
	printf("Options:\n");
	printf("  -rmin <value>   Set the minimum real part (default: -2.0)\n");
	printf("  -rmax <value>   Set the maximum real part (default: 1.0)\n");
	printf("  -imin <value>   Set the minimum imaginary part (default: -1.5)\n");
	printf("  -imax <value>   Set the maximum imaginary part (default: 1.5)\n");
	printf("  -w <value>      Set the image width (default: 1920)\n");
	printf("  -h <value>      Set the image height (default: 1080)\n");
	printf("  -b <value>      Set the block size (default: 8)\n");
	printf("  -t <value>      Set the number of threads (default: 4)\n");
	printf("  -it <value>      Set the maximum iterations (default: 1000)\n");
	printf("  -bs <value>     Set the buffer size value (default: 512)\n");
	printf("  -model <value>  Set the parallel model (0: dynamic, 1: tasking, 2: integrated, default: 0)\n");
	printf("  -help           Display this help message\n");
}

//Matrix
void runMatrixDeterminant(int argc, char** argv) {
	SettingsMatrix settings;
	settings.size = 10;
	settings.vandermonde = 1;
	settings.model = 0; // 0 - dynamic, 1 - tasking, 2 - integrated
	settings.prefab = 0; // 1 - 4x4, 2 - 10x10
	settings.thread_num = 4;
	settings.buffer_size = 512;

	double** matrix = NULL;
	
	#ifdef _DEBUG
		long double correctDet = 0;
		long double det = 0;
		double rel_error = 0;
	#endif
	// Parse command line arguments
	for (int i = 0; i < argc; i += 2) {
		if (!strcmp(argv[i], "-size")) {
			settings.size = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-vm")) {
			if (settings.prefab == 0) {
				settings.vandermonde = atoi(argv[i + 1]);
			}
			else {
				printf("Vandermonde option is not available for prefab matrices.\n");
				exit(0);
			}
		}
		else if (!strcmp(argv[i], "-model")) {
			settings.model = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-prefab")) {
			if (settings.vandermonde == 1) {
				printf("Prefab option is not available for Vandermonde matrices.\n");
				exit(0);
			}
			else {
				settings.prefab = atoi(argv[i + 1]);
			}		
		}
		else if (!strcmp(argv[i], "-t")) {
			settings.thread_num = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-bs")) {
			settings.buffer_size = atoi(argv[i + 1]);
		}
		else {
			printf("Invalid argument: %s\n", argv[i]);
			exit(0);
		}
	}
	//Display the settings (once)
	#ifdef _DEBUG
		displayMatrixSettings(settings);
	#endif
	//Generate matrix to calculate and instantly determine the correct determinant
	//if the vandermonde flag is set (default)
	if (settings.vandermonde) {
		matrix = generateVandermondeMatrix(settings.size);
		#ifdef _DEBUG
			correctDet = vandermondeDeterminant(settings.size);
		#endif
	}
	else {
		//Save from first tests 
		if (settings.prefab == 1) {
			settings.size = 4;
			matrix = generateMatrix(settings.size);
			#ifdef _DEBUG
				correctDet = -89.0;
			#endif

			matrix[0][0] = 2.0;  matrix[0][1] = 1.0;  matrix[0][2] = -1.0; matrix[0][3] = 2.0;
			matrix[1][0] = -3.0; matrix[1][1] = -1.0; matrix[1][2] = 2.0;  matrix[1][3] = -11.0;
			matrix[2][0] = -2.0; matrix[2][1] = 1.0;  matrix[2][2] = 2.0;  matrix[2][3] = -3.0;
			matrix[3][0] = 1.0;  matrix[3][1] = 2.0;  matrix[3][2] = 3.0;  matrix[3][3] = 4.0;
		}
		else if (settings.prefab == 2) {
			settings.size = 10;
			matrix = generateMatrix(settings.size);
			#ifdef _DEBUG
				correctDet = -4683298;
			#endif
			

			matrix[0][0] = 5.0;  matrix[0][1] = 2.0;  matrix[0][2] = 1.0;  matrix[0][3] = 4.0;  matrix[0][4] = 3.0;
			matrix[0][5] = 7.0;  matrix[0][6] = 2.0;  matrix[0][7] = 8.0;  matrix[0][8] = 1.0;  matrix[0][9] = 3.0;
			matrix[1][0] = 2.0;  matrix[1][1] = 6.0;  matrix[1][2] = 4.0;  matrix[1][3] = 7.0;  matrix[1][4] = 9.0;
			matrix[1][5] = 1.0;  matrix[1][6] = 3.0;  matrix[1][7] = 4.0;  matrix[1][8] = 5.0;  matrix[1][9] = 2.0;
			matrix[2][0] = 1.0;  matrix[2][1] = 4.0;  matrix[2][2] = 7.0;  matrix[2][3] = 3.0;  matrix[2][4] = 6.0;
			matrix[2][5] = 2.0;  matrix[2][6] = 8.0;  matrix[2][7] = 1.0;  matrix[2][8] = 4.0;  matrix[2][9] = 5.0;
			matrix[3][0] = 4.0;  matrix[3][1] = 7.0;  matrix[3][2] = 3.0;  matrix[3][3] = 8.0;  matrix[3][4] = 2.0;
			matrix[3][5] = 5.0;  matrix[3][6] = 6.0;  matrix[3][7] = 3.0;  matrix[3][8] = 2.0;  matrix[3][9] = 4.0;
			matrix[4][0] = 3.0;  matrix[4][1] = 9.0;  matrix[4][2] = 6.0;  matrix[4][3] = 2.0;  matrix[4][4] = 4.0;
			matrix[4][5] = 7.0;  matrix[4][6] = 1.0;  matrix[4][7] = 5.0;  matrix[4][8] = 3.0;  matrix[4][9] = 6.0;
			matrix[5][0] = 7.0;  matrix[5][1] = 1.0;  matrix[5][2] = 2.0;  matrix[5][3] = 5.0;  matrix[5][4] = 7.0;
			matrix[5][5] = 4.0;  matrix[5][6] = 9.0;  matrix[5][7] = 8.0;  matrix[5][8] = 2.0;  matrix[5][9] = 5.0;
			matrix[6][0] = 2.0;  matrix[6][1] = 3.0;  matrix[6][2] = 8.0;  matrix[6][3] = 6.0;  matrix[6][4] = 1.0;
			matrix[6][5] = 9.0;  matrix[6][6] = 4.0;  matrix[6][7] = 2.0;  matrix[6][8] = 5.0;  matrix[6][9] = 6.0;
			matrix[7][0] = 8.0;  matrix[7][1] = 4.0;  matrix[7][2] = 1.0;  matrix[7][3] = 3.0;  matrix[7][4] = 5.0;
			matrix[7][5] = 2.0;  matrix[7][6] = 8.0;  matrix[7][7] = 6.0;  matrix[7][8] = 1.0;  matrix[7][9] = 3.0;
			matrix[8][0] = 1.0;  matrix[8][1] = 5.0;  matrix[8][2] = 4.0;  matrix[8][3] = 2.0;  matrix[8][4] = 3.0;
			matrix[8][5] = 6.0;  matrix[8][6] = 2.0;  matrix[8][7] = 1.0;  matrix[8][8] = 5.0;  matrix[8][9] = 7.0;
			matrix[9][0] = 3.0;  matrix[9][1] = 2.0;  matrix[9][2] = 5.0;  matrix[9][3] = 4.0;  matrix[9][4] = 6.0;
			matrix[9][5] = 5.0;  matrix[9][6] = 6.0;  matrix[9][7] = 3.0;  matrix[9][8] = 7.0;  matrix[9][9] = 9.0;
		}
		else {
			printf("Prefab no. %d not implemented\n", settings.prefab);
		}
	}
	if (settings.vandermonde || settings.prefab) {
		//Run calculations based on the selected model
		switch (settings.model) {
		case 0: dynamicForMatrixDeterminant(matrix, settings); break;
		case 1: taskingMatrixDeterminant(matrix, settings); break;
		case 2: integratedMasterMatrixDeterminant(matrix, settings); break;
		default: printf("Bad model\n"); break;
		}
		#ifdef _DEBUG
			// Calculate the determinant
			det = determinant(matrix);
			// Check the relative error
			rel_error = fabs(det - correctDet) / fabs(correctDet);
		#endif
	}
	
	// Free the result buffer
	if (matrix != NULL) {
		for (int i = 0; i < settings.size; i++) {
			if (matrix[i] != NULL) {
				free(matrix[i]);
			}
		}
		free(matrix);
	}
	#ifdef _DEBUG
		//print_progress(100);
		//print results (for vandermonde determinand increases very very VERY fast)
		printf("\nMy det: %Lf - Correct det: %Lf\n", det, correctDet);
		printf("Relative error: %.10e\n", rel_error);
		printf("Matrix determination calculation completed.\n");
	#endif
}
void displayMatrixSettings(SettingsMatrix settings) {
	const char* model_name;

	switch (settings.model) {
	case 0: model_name = "dynamic"; break;
	case 1: model_name = "tasking"; break;
	case 2: model_name = "integrated"; break;
	default: model_name = "unknown"; break;
	}

	printf("----- Settings -----\n");
	printf("Size		: %d x %d\n", settings.size, settings.size);
	printf("Vandermonde	: %d\n", settings.vandermonde);
	printf("Prefab		: %d\n", settings.prefab);
	printf("Thread count    : %d\n", settings.thread_num);
	printf("Buffer size     : %d\n", settings.buffer_size);
	printf("Parallel model  : %s\n", model_name);
	printf("--------------------\n");
}
void displayMatrixHelp() {
	printf("Usage: MatrixDeterminant [options]\n");
	printf("Options:\n");
	printf("  -size <value>   Set the size of the matrix (default: 10)\n");
	printf("  -vm <value>     Set the Vandermonde matrix flag (default: 1)\n");
	printf("  -model <value>   Set the parallel model (0: dynamic, 1: tasking, 2: integrated, default: 0)\n");
	printf("  -prefab <value>  Set the prefab matrix flag (default: 0)\n");
	printf("  -t <value>      Set the number of threads (default: 4)\n");
	printf("  -bs <value>     Set the buffer size value (default: 512)\n");
	printf("  -help           Display this help message\n");
}
double** generateMatrix(int size) {
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
	return matrix;
}

// Merge Sort
void runMergeSort(int argc, char** argv) {
	SettingsSort settings;
	settings.size = 1024;
	settings.model = 0; // 0 - dynamic, 1 - tasking, 2 - integrated
	settings.thread_num = 4;
	settings.buffer_size = 512;

	for (int i = 0; i < argc; i += 2) {
		if (!strcmp(argv[i], "-size")) {
			settings.size = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-t")) {
			settings.thread_num = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-model")) {
			settings.model = atoi(argv[i + 1]);
		}
		else if (!strcmp(argv[i], "-bs")) {
			settings.buffer_size = atoi(argv[i + 1]);
		}
		else {
			printf("Invalid argument: %s\n", argv[i]);
			exit(0);
		}
	}
	// Display the settings (once)
	#ifdef _DEBUG
		displayMergeSortSettings(settings);
	#endif
	// Allocate memory for the array (reverse order) ## worst case ##
	int* array = generateArray(settings.size);
	g_array = array;
	// Run Merge Sort based on the selected model
	switch (settings.model) {
	case 0: dynamicForMergeSort(array, settings); break;
	case 1: taskingMergeSort(array, settings); break;
	case 2: integratedMasterMergeSort(array, settings); break;
	default: printf("Bad model\n"); break;
	}
	#ifdef _DEBUG
		print_progress(100);

		//reverse order should now be sorted
		printf("\nTesting correction...\n");
		int flag = 0;
		for (int i = 0; i < arraySize; i++) {
			if (array[i] != i + 1) {
				printf("Error: should be %d, is %d\nTesting terminated\n", i + 1, array[i]);
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			printf("Array sorted correctly.\n");
		}
		else {
			printf("Array not sorted correctly.\n");
		}
	#endif
	free(array);
}
void displayMergeSortSettings(SettingsSort settings) {
	const char* model_name;

	switch (settings.model) {
	case 0: model_name = "dynamic"; break;
	case 1: model_name = "tasking"; break;
	case 2: model_name = "integrated"; break;
	default: model_name = "unknown"; break;
	}
	// Display the settings
	printf("----- Settings -----\n");
	printf("Size		: %d\n", settings.size);
	printf("Thread count    : %d\n", settings.thread_num);
	printf("Buffer size     : %d\n", settings.buffer_size);
	printf("Parallel model  : %s\n", model_name);
	printf("--------------------\n");
}
void displayMergeSortHelp() {
	printf("Usage: MergeSort [options]\n");
	printf("Options:\n");
	printf("  -size <value>   Set the size of the array (default: 1000)\n");
	printf("  -model <value>  Set the parallel model (0: dynamic, 1: tasking, 2: integrated, default: 0)\n");
	printf("  -t <value>      Set the number of threads (default: 4)\n");
	printf("  -bs <value>     Set the buffer size value (default: 512)\n");
	printf("  -help           Display this help message\n");
}
int* generateArray(int size) {
	int* array = (int*)malloc(sizeof(int) * size);
	if (array == NULL) {
		perror("Memory allocation failed (array)");
		exit(EXIT_FAILURE);
	}
	for (int i = size; i > 0; i--) {
		array[size - i] = i;
	}
	return array;
}

