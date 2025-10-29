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

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>
#include <math.h>

int computecoeff=1;

int threadnum=4; // should be at least 2 - master and 1+ slave(s)
long CHUNKCOUNT=100000;

#define BUFFERSIZE 512

typedef struct {
  double a;
  double b;
} t_input;

typedef struct {
  double r;
} t_output;

typedef struct {
  double r;
} t_final_output;

void init_final_output(t_final_output *output) {
  output->r=0;
}

double f(double x) {
  return x*sin(x*x)*sin(x*x);
  
  //  return sin(x);
}

long generate_new_input(t_input *input) { // returned value: how many items generated
  static double a=0;
  static int howmanygenerated=0;
  int counter;
  int max=BUFFERSIZE;
  if (BUFFERSIZE>(CHUNKCOUNT-howmanygenerated))
    max=CHUNKCOUNT-howmanygenerated;
  for(counter=0;counter<max;counter++) {
    input[counter].a=a;
    a+=100.0/(double)CHUNKCOUNT;
    input[counter].b=a;
    howmanygenerated++;
  }
  return counter;
}

/*
t_output process(t_input *data) {
  long stepcount=100000/computecoeff;
  
  t_output result;
  double a=data->a;
  double step=(data->b-data->a)/(double)stepcount;

  result.r=0;

  for(int i=0;i<stepcount;i++) {
    result.r+=(f(a)*step);
    a+=step;
  }

  return result;
}
*/

t_output process(t_input *data) {

  t_output result;
  double a=data->a;
  double b=data->b;
  double c=(a+b)/2;

  // compute the area of a triangle
  double l1=sqrt((b-a)*(b-a)+(f(b)-f(a))*(f(b)-f(a)));
  double l2=sqrt((c-a)*(c-a)+(f(c)-f(a))*(f(c)-f(a)));
  double l3=sqrt((c-b)*(c-b)+(f(c)-f(b))*(f(c)-f(b)));
  double p=(l1+l2+l3)/2;
  
  double trianglearea=sqrt(p*(p-l1)*(p-l2)*(p-l3));

  if (trianglearea<(0.000000000000000001/computecoeff)) { // satisfactory accuracy

    result.r=((f(a)+f(c))*(c-a)+(f(c)+f(b))*(b-c))/2;

  } else {
    t_input x1,x2;
    x1.a=a; x1.b=c;
    x2.a=c; x2.b=b;
    t_output y1,y2;
    y1=process(&x1);
    y2=process(&x2);
    
    result.r=y1.r+y2.r;
  }

  return result;
}


void merge(t_final_output *final,t_output *current) {
  final->r+=current->r;
}

void print_final_output(t_final_output *final) {
  printf("Result=%f",final->r);
  fflush(stdout);
}

int main(int argc,char **argv) {

  t_input input[BUFFERSIZE]; // firstly some input data is generated, then the master adds some data
  long currentinputindex=0; // points to the next item to fetch
  t_output output[BUFFERSIZE]; // threads put results into this array (can be in a different order than the input)
  long currentoutputcount=0; // how many results are currently in the output buffer

  t_final_output finaloutput;
  int i;
  long lastgeneratedcount; // how many items were generated last time
  int threadnumber;
  omp_lock_t inputoutputlock;

  long processedcount=0; // should finally reach CHUNKCOUNT

  if (argc>1)
    threadnum=atoi(argv[1]);

  if (argc>2) {
    computecoeff=atoi(argv[2]); // defines the compute coefficient
    //CHUNKCOUNT*=computecoeff;
  }

  
  omp_init_lock(&inputoutputlock);

  // firstly generate BUFFERSIZE data chunks of input data

  lastgeneratedcount=generate_new_input(input);

  init_final_output(&finaloutput);

#pragma omp parallel private(i) shared(input,output,finaloutput,currentinputindex,currentoutputcount,lastgeneratedcount,processedcount) num_threads(threadnum)
  {
    // each thread acts as a slave

    int processdata;
    t_output result;
    long myinputindex;      
    int finish;
    do {
      processdata=0;    
      finish=0;
      omp_set_lock(&inputoutputlock);
      if (processedcount<CHUNKCOUNT) {
        myinputindex=currentinputindex;
        if (currentinputindex<lastgeneratedcount) {
    	  currentinputindex++;
	  processdata=1;
        }
      } else finish=1;
      omp_unset_lock(&inputoutputlock);
      
      if (processdata) {
	// now process the input data chunk
	result=process(&(input[myinputindex]));
	
	// store the result in the output buffer
	omp_set_lock(&inputoutputlock);
	if (currentoutputcount<BUFFERSIZE) {
	  output[currentoutputcount]=result;
	  currentoutputcount++;
	}
	if (currentoutputcount==lastgeneratedcount) { // process all available results
	  for(i=0;i<lastgeneratedcount;i++)
	    merge(&finaloutput,&(output[i]));
	  processedcount+=lastgeneratedcount;
	  currentoutputcount=0;

	  if (processedcount<CHUNKCOUNT) { 
	    lastgeneratedcount=generate_new_input(input);
	    currentinputindex=0;
	  }
	}
	omp_unset_lock(&inputoutputlock);
      }      
    } while (!finish);
  }
  

  print_final_output(&finaloutput);
  
  omp_destroy_lock(&inputoutputlock);
}
