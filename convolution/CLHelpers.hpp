#ifndef __CLHELPERS_H__
#define __CLHELPERS_H__

#include <CL/cl.hpp>

#include <vector>

class CLHelpers
{
public:

  static void getDevice(int id, cl::Device& device);
  static void getAllDevices(std::vector<cl::Device>& devices);
  static void getContextDevices(const cl::Context& context, std::vector<cl::Device>& devices);

  static void printDeviceInfo(const cl::Device& device);
  static void printDevicesInfo(const std::vector<cl::Device>& devices);
  static void printAllDeviceInfo();

  static void printKernelInfo(const cl::Kernel& kernel, const cl::Device& device);

  static void printPlatformInfo(const cl::Platform& platform);
  static void printPlatformsInfo(const std::vector<cl::Platform>& platforms);
};

#endif
