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

#ifndef SRC_HSA_HIST_HSA_HIST_BENCHMARK_H_
#define SRC_HSA_HIST_HSA_HIST_BENCHMARK_H_

#include "src/common/benchmark/benchmark.h"

class HistBenchmark : public Benchmark {
 private:
	bool random_ = false;
  unsigned int local_size_ = 256;
  unsigned int global_size_ = 256;
  unsigned int num_pixels_ = 65536;
	unsigned int num_colors_ = 256;
  unsigned int* input_ = nullptr;
  unsigned int* output_ = nullptr;
 public:
  void Initialize() override;
  void Run() override;
  void Verify() override;
  void Summarize() override;
  void Cleanup() override;

  void setNumPixels(unsigned int pixels) { this->num_pixels_ = pixels; }
	void setRandom(bool random) { this->random_ = random; }
	void setColors(unsigned int colors) { this->num_colors_ = colors; }
};

#endif  // SRC_HSA_HIST_HSA_HIST_BENCHMARK_H_
