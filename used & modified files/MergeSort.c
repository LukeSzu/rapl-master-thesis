#include "MergeSort.h"

int BUFFERSIZE;
int arraySize;
int CHUNKCOUNT;

long generate_new_input_sort(t_input_sort* input, int* array) { // returned value: how many items generated

	static int level = 1; //level of recursion for example level 1 has 2^1=2 elements in input
    static int howmanygenerated = 0;
	static int done_phases_this_level = 0; //only one layer of recursion is done at a time
    int pow = 1 << level;
	int total_phases_this_level = arraySize / pow; //max number of phases in this level
	int forced = arraySize - pow * total_phases_this_level; //how many elements are left to be sorted and has to be added to the last input


    int counter = 0;
	//maximum number of elements to be generated this time
    int max = BUFFERSIZE;
    if (BUFFERSIZE > (CHUNKCOUNT - howmanygenerated))
        max = CHUNKCOUNT - howmanygenerated;

    for (counter = 0;counter < max;counter++) {
		//this is for the last phase of the last level
        if (pow >= arraySize) {
            input[counter].left = array;
            input[counter].middle = arraySize / 2 - 1;
            input[counter].size = arraySize;
            input[counter].force = 0;
        }
        //standard case
        else {
            input[counter].left = &(array[(1 << level) * done_phases_this_level]);
            input[counter].middle = (1 << (level - 1)) - 1;
            input[counter].size = (1 << level);
            input[counter].force = 0;
        }
        howmanygenerated++;
        done_phases_this_level++;
        //if there is remaining elements, add them to the last input
        #ifdef _DEBUG
            int input_left_idx = input[counter].left - array; 
            int input_size = input[counter].size;
            int input_middle = input[counter].middle;
            int input_force = input[counter].force;
        #endif
        if (done_phases_this_level == total_phases_this_level) {
            if (forced) {
                input[counter].force = forced;
                #ifdef _DEBUG
                    input_force = input[counter].force;
                #endif
            }
            counter++;
            #ifdef _DEBUG
                FILE* fp = fopen("debug.txt", "a");
                fprintf(fp, "left: %d, middle: %d, size: %d, force: %d, level: %d, done_phases_this_level: %d, total_phases_this_level: %d, pow: %d, forced: %d\n", input_left_idx, input_middle, input_size, input_force, level, done_phases_this_level, total_phases_this_level, pow, forced);
                fclose(fp);
            #endif
            break;
        }
        #ifdef _DEBUG
            FILE* fp = fopen("debug.txt", "a");
            fprintf(fp, "left: %d, middle: %d, size: %d, force: %d, level: %d, done_phases_this_level: %d, total_phases_this_level: %d, pow: %d, forced: %d\n", input_left_idx, input_middle, input_size, input_force, level, done_phases_this_level, total_phases_this_level, pow, forced);
            fclose(fp);
        #endif
    }
	//if level is done, go to the next level
    if (done_phases_this_level >= total_phases_this_level) {
        level++;
        done_phases_this_level = 0;

    }
	
    return counter;

}

void process_sort(t_input_sort* data) {

	//temporary array for merging
    int* temp = (int*)malloc(sizeof(int) * data->size);
    if (temp == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    //sorting
    int i = 0, j = data->middle + 1, k = 0;
    while (i <= data->middle && j < data->size) {
        if (data->left[i] < data->left[j]) {
            temp[k++] = data->left[i++];
        }
        else {
            temp[k++] = data->left[j++];
        }
    }
	//copying the rest of the elements
    while (i <= data->middle) temp[k++] = data->left[i++];
    while (j < data->size) temp[k++] = data->left[j++];

	//copying the sorted elements back to the original array
    for (i = 0; i < data->size; i++) {
        data->left[i] = temp[i];
    }
	//free the temporary array
    free(temp);
    //Do it again with the remaining elements
	//This is called only when the array size is not a power of two and only in the last input
    //This is called max once per level
    if (data->force) {
        data->middle = data->size - 1;
        data->size = data->size + data->force;
        data->force = 0;
        process_sort(data);
    }
}

