#ifndef PARAMS_H_
#define PARAMS_H_

#include "CLHelpers.hpp"

#include <vector>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_NUM_THREADS 4

struct paramStruct
{
  int nWidth;		// Output image width
  int nHeight;		// Output image height
  int nInWidth;		// Input  image width
  int nInHeight;	// Input  image height
  int nFilterWidth;	// Filter size is nFilterWidth X nFilterWidth
  int nIterations;	// Run timing loop for nIterations

  int nMode;		// Execution mode (-1=All, 0=CPU, 1=GPU)

  // Test CPU performance with 1,4,8 etc. OpenMP threads
  std::vector<int> ompThreads;
  int nOmpRuns;		// ompThreads.size()

  bool benchmark;	// Benchmark mode

} params;

void Usage(char *name);
void ParseCommandLine(int argc, char* argv[]);

void InitParams(int argc, char* argv[])
{
  params.nWidth = 1024;
  params.nHeight = 1024;
  params.nFilterWidth = 3;
  params.nIterations = 1;

  params.nMode = -1;

  params.benchmark = false;

  ParseCommandLine(argc, argv);

  params.nInWidth = params.nWidth + (params.nFilterWidth-1);
  params.nInHeight = params.nHeight + (params.nFilterWidth-1);

  params.ompThreads.push_back(4);
  //params.ompThreads.push_back(1);
  //params.ompThreads.push_back(8);
  params.nOmpRuns = params.ompThreads.size();
}

void ParseCommandLine(int argc, char* argv[])
{
  for (int i = 1; i < argc; ++i)
  {
    switch (argv[i][1])
    {
    case 'm':
      if (++i < argc)
	sscanf(argv[i], "%d", &params.nMode);
      break;
    case 'p':
      CLHelpers::printAllDeviceInfo();
      exit(EXIT_SUCCESS);
    case 'b':
      params.benchmark = true;
      break;
    case 'f':
      if (++i < argc)
      {
	sscanf(argv[i], "%u", &params.nFilterWidth);
      }
      else
      {
	std::cerr << "Could not read argument after option " << argv[i-1] << std::endl;
	Usage(argv[0]);
	throw;
      }
      break;
    case 'i':
      if (++i < argc)
      {
	sscanf(argv[i], "%u", &params.nIterations);
      }
      else
      {
	std::cerr << "Could not read argument after option " << argv[i-1] << std::endl;
	Usage(argv[0]);
	throw;
      }
      break;
    case 'x':
      if (++i < argc)
      {
	sscanf(argv[i], "%u", &params.nWidth);
      }
      else
      {
	std::cerr << "Could not read argument after option " << argv[i-1] << std::endl;
	Usage(argv[0]);
	throw;
      }
      break;
    case 'y':
      if (++i < argc)
      {
	sscanf(argv[i], "%u", &params.nHeight);
      }
      else
      {
	std::cerr << "Could not read argument after option " << argv[i-1] << std::endl;
	Usage(argv[0]);
	throw;
      }
      break;
    case 'h':
      Usage(argv[0]);
      exit(1);
    default:
      std::cerr << "Invalid argument " << argv[i] << std::endl;
      Usage(argv[0]);
      throw(std::string("Invalid argument"));
    }
  }
}

void Usage(char *name)
{
  printf("\tUsage: %s [-h] [-m <int>] [-p] [-b] [-f <int>] [-i <int>] [-x <int>] [-y <int>]\n", name);
  printf("   -h		Print this help menu.\n");
  printf("   -m <int>	Mode (0=CPU, 1=GPU).\n");
  printf("   -p		Print available OpenCL platforms.\n");
  printf("   -b		Benchmark mode.\n");
  printf("   -f <int>	Sets the filter width.\n");
  printf("   -i <int>	Number of iterations.\n");
  printf("   -x <int>	Sets the image width.\n");
  printf("   -y <int>	Sets the image height.\n");
}


#endif    //#ifndef PARAMS_H_
