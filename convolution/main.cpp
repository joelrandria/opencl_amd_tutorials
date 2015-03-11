#define __CL_ENABLE_EXCEPTIONS

#include "Convolution.hpp"
#include "Params.hpp"

#include "util.hpp"
#include <CL/cl.hpp>

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
// Host buffers
/////////////////////////////////////////////////////////////////

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

  cout << "Mode:           ";
  switch (params.nMode)
  {
  case -1: cout << "All device" << endl; break;
  case 0: cout << "CPU" << endl; break;
  case 1: cout << "GPU" << endl; break;
  }

  cout << "Testing:        ";

  if (params.nMode < 1)
    for (int run = 0; run < params.nOmpRuns; run++)
      cout << "CPU (" << params.ompThreads[run] << "-threads) , ";

  cout << endl << endl;
}

void PrintCPUTime(int run)
{
  if (params.nMode < 1)
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

void RunCPU(int ompThreadCount)
{
  cout << "\n********    Starting CPU (" << ompThreadCount << "-threads) run    ********" << endl;

  if (!params.benchmark)
  {
    timers.counter.Reset();
    timers.counter.Start();

    for (int i = 0; i < params.nIterations; i++)
      Convolve(hostBuffers.pInput, hostBuffers.pFilter, hostBuffers.pOutputCPU,
	       params.nInWidth,
	       params.nWidth, params.nHeight,
	       params.nFilterWidth,
	       ompThreadCount);

    timers.counter.Stop();
    timers.dCpuTime = timers.counter.GetElapsedTime()/double(params.nIterations);

    PrintCPUTime(ompThreadCount);
  }
  else
  {
    for (int j = 0; j < BENCHMARK_FILTER_COUNT; ++j)
    {
      InitFilterHostBuffer(benchmarkFilterWidths[j]);

      timers.counter.Reset();
      timers.counter.Start();

      for (int i = 0; i < params.nIterations; i++)
	Convolve(hostBuffers.pInput, hostBuffers.pFilter, hostBuffers.pOutputCPU,
		 params.nInWidth,
		 params.nWidth, params.nHeight,
		 benchmarkFilterWidths[j],
		 ompThreadCount);

      timers.counter.Stop();
      timers.dCpuTime = timers.counter.GetElapsedTime()/double(params.nIterations);

      stats.cpu4Threads.add(benchmarkFilterWidths[j], timers.dCpuTime);

      cout << "Filter size = " << benchmarkFilterWidths[j] << ": CPU time = " << timers.dCpuTime << "s" << endl;
    }
  }
}

/////////////////////////////////////////////////////////////////
// Convolution on GPU
/////////////////////////////////////////////////////////////////

#define CONVOLUTION_CL_FILENAME "convolution.cl"

void RunGPU()
{
  std::vector<cl::Device> devices;
  const int targetDevice = 1;

  cl::Context context;
  cl::CommandQueue queue;
  cl::Program program;

  CLHelpers::getAllDevices(devices);

  context = cl::Context(devices[targetDevice]);
  queue = cl::CommandQueue(context, devices[targetDevice]);

  try
  {
    program = cl::Program(context, util::loadProgram(CONVOLUTION_CL_FILENAME));
    program.build();
  }
  catch (cl::Error e)
  {
    std::string log;

    program.getBuildInfo(devices[targetDevice], CL_PROGRAM_BUILD_LOG, &log);

    fprintf(stderr, "Exception: %s\r\n", e.what());
    fprintf(stderr, "\r\n%s\r\n", log.c_str());

    exit(EXIT_FAILURE);
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

    switch (params.nMode)
    {
    case -1:
      RunCPU(4);
      RunGPU();
      break;
    case 0:
      RunCPU(4);
      break;
    case 1:
      RunGPU();
      break;
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
