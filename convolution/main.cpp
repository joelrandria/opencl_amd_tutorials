#include "Timer.h"
#include "Params.h"
#include "StatFile.h"

#include <omp.h>
#include <string>
#include <iomanip>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::setw;
using std::string;

/////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////

#define FREE(ptr, free_val)			\
  if (ptr != free_val)				\
  {						\
    free(ptr);					\
    ptr = free_val;				\
  }

/////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////

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

#define FILTER_WIDTH_COUNT 5

int filterWidths[FILTER_WIDTH_COUNT] = {2, 4, 8, 16, 32};

/////////////////////////////////////////////////////////////////
// Host buffers
/////////////////////////////////////////////////////////////////

void InitFilterHostBuffer(int width);

void InitHostBuffers()
{
  hostBuffers.pInput  = NULL;
  hostBuffers.pOutputCPU = NULL;
  hostBuffers.pFilter = NULL;

  /////////////////////////////////////////////////////////////////
  // Allocate and initialize memory used by host
  /////////////////////////////////////////////////////////////////
  int sizeInBytes = params.nInWidth * params.nInHeight * sizeof(float);
  hostBuffers.pInput = (float *) malloc(sizeInBytes);
  if (!hostBuffers.pInput)
    throw(string("InitHostBuffers()::Could not allocate memory"));

  int sizeOutBytes = params.nWidth * params.nHeight * sizeof(float);
  hostBuffers.pOutputCPU = (float *) malloc(sizeOutBytes);
  if (!hostBuffers.pOutputCPU)
    throw(string("InitHostBuffers()::Could not allocate memory"));

  srand(0);
#pragma omp parallel for num_threads(DEFAULT_NUM_THREADS)
  for (int i = 0; i < params.nInWidth * params.nInHeight; i++)
  {
    hostBuffers.pInput[i] = float(rand());
  }

  InitFilterHostBuffer(params.nFilterWidth);
}
void InitFilterHostBuffer(int width)
{
  if (hostBuffers.pFilter)
    FREE(hostBuffers.pFilter, NULL);

  int filterSizeBytes = width * width * sizeof(float);
  hostBuffers.pFilter = (float *) malloc(filterSizeBytes);
  if (!hostBuffers.pFilter)
    throw(string("InitFilterHostBuffer()::Could not allocate memory"));

  double dFilterSum = 0;
  int nFilterSize = width * width;
  for (int i = 0; i < nFilterSize; i++)
  {
    hostBuffers.pFilter[i] = float(rand());
    dFilterSum += hostBuffers.pFilter[i];
  }
  for (int i = 0; i < nFilterSize; i++)
    hostBuffers.pFilter[i] /= dFilterSum;
}

void ClearBuffer(float * pBuf)
{
#pragma omp parallel for num_threads(DEFAULT_NUM_THREADS)
  for (int i = 0; i < params.nWidth*params.nHeight; i++)
  {
    pBuf[i] = -999.999f;
  }
}

void ReleaseHostBuffers()
{
  FREE(hostBuffers.pInput, NULL);
  FREE(hostBuffers.pOutputCPU, NULL);
  FREE(hostBuffers.pFilter, NULL);
}

/////////////////////////////////////////////////////////////////
// Print info, timing
/////////////////////////////////////////////////////////////////

void PrintInfo()
{
  cout << endl;
  cout << "Width:          " << params.nWidth << endl;
  cout << "Height:         " << params.nHeight << endl;
  cout << "Filter Size:    " << params.nFilterWidth << " x "
       << params.nFilterWidth << endl;
  cout << "Iterations:     " << params.nIterations << endl;
  cout << "CPU Timing:     " << (params.bCPUTiming ? "True":"False") << endl;
  cout << "Testing:        ";

  if (params.bCPUTiming)
    for (int run = 0; run < params.nOmpRuns; run++)
      cout << "CPU (" << params.ompThreads[run] << "-threads) , ";

  cout << endl << endl;
}

void PrintCPUTime(int run)
{
  if (params.bCPUTiming)
    cout << "CPU (" << params.ompThreads[run] << "-threads): " << timers.dCpuTime << endl;
}

/////////////////////////////////////////////////////////////////
// Statistics
/////////////////////////////////////////////////////////////////

void InitStatFiles()
{
  StatFile::clearDirectory("data");

  stats.cpu4Threads.open("data/cpu_4_threads.dat");
}
void ReleaseStatFiles()
{
  stats.cpu4Threads.close();
}

/////////////////////////////////////////////////////////////////
// Convolution on CPU
/////////////////////////////////////////////////////////////////

void Convolve(float * pInput, float * pFilter, float * pOutput,
	      const int nInWidth, const int nWidth, const int nHeight,
	      const int nFilterWidth, const int nNumThreads)
{
#pragma omp parallel for num_threads(nNumThreads)
  for (int yOut = 0; yOut < nHeight; yOut++)
  {
    const int yInTopLeft = yOut;

    for (int xOut = 0; xOut < nWidth; xOut++)
    {
      const int xInTopLeft = xOut;

      float sum = 0;
      for (int r = 0; r < nFilterWidth; r++)
      {
	const int idxFtmp = r * nFilterWidth;

	const int yIn = yInTopLeft + r;
	const int idxIntmp = yIn * nInWidth + xInTopLeft;

	for (int c = 0; c < nFilterWidth; c++)
	{
	  const int idxF  = idxFtmp  + c;
	  const int idxIn = idxIntmp + c;
	  sum += pFilter[idxF]*pInput[idxIn];
	}
      } //for (int r = 0...

      const int idxOut = yOut * nWidth + xOut;
      pOutput[idxOut] = sum;

    } //for (int xOut = 0...
  } //for (int yOut = 0...
}

void RunCPU(int run)
{
  if (params.bCPUTiming)
  {
    cout << "\n********    Starting CPU (" << params.ompThreads[run] << "-threads) run    ********" << endl;

    if (!params.benchmark)
    {
      timers.counter.Reset();
      timers.counter.Start();

      for (int i = 0; i < params.nIterations; i++)
	Convolve(hostBuffers.pInput, hostBuffers.pFilter, hostBuffers.pOutputCPU,
		 params.nInWidth,
		 params.nWidth, params.nHeight,
		 params.nFilterWidth,
		 params.ompThreads[run]);

      timers.counter.Stop();
      timers.dCpuTime = timers.counter.GetElapsedTime()/double(params.nIterations);

      PrintCPUTime(run);
    }
    else
    {
      for (int j = 0; j < FILTER_WIDTH_COUNT; ++j)
      {
	InitFilterHostBuffer(filterWidths[j]);

	timers.counter.Reset();
	timers.counter.Start();

	for (int i = 0; i < params.nIterations; i++)
	  Convolve(hostBuffers.pInput, hostBuffers.pFilter, hostBuffers.pOutputCPU,
		   params.nInWidth,
		   params.nWidth, params.nHeight,
		   filterWidths[j],
		   params.ompThreads[run]);

	timers.counter.Stop();
	timers.dCpuTime = timers.counter.GetElapsedTime()/double(params.nIterations);

	stats.cpu4Threads.add(filterWidths[j], timers.dCpuTime);

	cout << "Filter size = " << filterWidths[j] << ": CPU time = " << timers.dCpuTime << "s" << endl;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  try
  {
    InitParams(argc, argv);
    PrintInfo();

    InitHostBuffers();
    InitStatFiles();

    for (int run = 0; run < params.nOmpRuns; run++)
    {
      ClearBuffer(hostBuffers.pOutputCPU);
      RunCPU(run);
    }

    ReleaseHostBuffers();
    ReleaseStatFiles();
  }
  catch(std::string msg)
  {
    cerr << "Exception caught in main(): " << msg << endl;
    ReleaseHostBuffers();
    ReleaseStatFiles();
  }
  catch(...)
  {
    cerr << "Exception caught in main()" << endl;
    ReleaseHostBuffers();
    ReleaseStatFiles();
  }

  return 0;
}
