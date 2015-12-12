/*
 * Hetero-mark
 *
 * Copyright (c) 2015 Northeastern University
 * All rights reserved.
 *
 * Developed by:
 *   Northeastern University Computer Architecture Research (NUCAR) Group
 *   Northeastern University
 *   http://www.ece.neu.edu/groups/nucar/
 *
 * Author: Alejandro Villegas (avillegas@uma.es)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal with the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 *   Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimers.
 *
 *   Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimers in the 
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the names of NUCAR, Northeastern University, nor the names of 
 *   its contributors may be used to endorse or promote products derived 
 *   from this Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS WITH THE SOFTWARE.
 */

#include <cstdlib>
#include <cstdio>
#include <ctime>  
#include "src/hsa/hist_hsa/kernels.h"
#include "src/hsa/hist_hsa/hist_benchmark.h"

void HistBenchmark::Initialize() {

  input_ = new unsigned int[num_pixels_];
  output_ = new unsigned int[num_colors_];

  // Initialize input data
	if (!(this->random_))
	{	
		for (unsigned int i = 0; i < num_pixels_; i++) 
		{    
			input_[i] = i%num_colors_;
		}
	}
	else
	{
		srand (time(NULL));	
		for (unsigned int i = 0; i < num_pixels_; i++) 
		{    
				input_[i] = rand()%num_colors_;
		}
	}

  // Initialize output
  for (unsigned int i = 0; i < (this->num_colors_); i++) {
    output_[i] = 0; 
  }
}

void HistBenchmark::Run() {
  SNK_INIT_LPARM(lparm, 0);
  lparm->ndim = 1;
  lparm->gdims[0] = global_size_;
  lparm->ldims[0] = local_size_;
  HIST(input_, output_, num_colors_, num_pixels_, lparm);
}

void HistBenchmark::Verify() {

	printf("output = \n");
  for (unsigned int i = 0; i < num_colors_; i++) {
    printf("%u ",output_[i]);
  }
	printf("\n");
}

void HistBenchmark::Summarize() {
}

void HistBenchmark::Cleanup() {
  delete[] input_;
  delete[] output_;
}
