#ifndef __CONVOLUTION_H__
#define __CONVOLUTION_H__

#include "Timer.h"
#include "StatFile.h"

struct hostBufferStruct
{
  float * pInput;
  float * pFilter;
  float * pOutputCPU;
} hostBuffers;

struct timerStruct
{
  double dCpuTime;
  CPerfCounter counter;
} timers;

struct statFileStruct
{
  StatFile cpu4Threads;
} stats;

#define BENCHMARK_FILTER_COUNT 6

int benchmarkFilterWidths[BENCHMARK_FILTER_COUNT] = {2, 4, 8, 16, 32, 64};

#define FREE(ptr, free_val)			\
  if (ptr != free_val)				\
  {						\
    free(ptr);					\
    ptr = free_val;				\
  }

/////////////////////////////////////////////////////////////////
// Host buffers
/////////////////////////////////////////////////////////////////

  void InitFilterHostBuffer(int width);
  void InitHostBuffers();
 void InitFilterHostBuffer(int width);

 void ClearBuffer(float * pBuf);
 void ReleaseHostBuffers();

/////////////////////////////////////////////////////////////////
// Print info, timing
/////////////////////////////////////////////////////////////////

void PrintInfo();
void PrintCPUTime(int run);

/////////////////////////////////////////////////////////////////
// Statistics
/////////////////////////////////////////////////////////////////

 void InitStatFiles();
 void ReleaseStatFiles();

/////////////////////////////////////////////////////////////////
// Convolution on CPU
/////////////////////////////////////////////////////////////////

void Convolve(float * pInput, float * pFilter, float * pOutput,
	      const int nInWidth, const int nWidth, const int nHeight,
	      const int nFilterWidth, const int nNumThreads);

void RunCPU(int run);

#endif
