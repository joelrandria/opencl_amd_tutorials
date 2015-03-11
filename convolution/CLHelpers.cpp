#include "CLHelpers.hpp"

#include <cstdio>

void CLHelpers::getDevice(int id, cl::Device& device)
{
  std::vector<cl::Device> devices;

  getAllDevices(devices);
  device = devices[id];
}
void CLHelpers::getAllDevices(std::vector<cl::Device>& devices)
{
  uint i;
  uint platformCount;

  std::vector<cl::Platform> platforms;
  std::vector<cl::Device> platformDevices;

  cl::Platform::get(&platforms);

  devices.clear();
  platformCount = platforms.size();

  for (i = 0; i < platformCount; ++i)
  {
    platformDevices.clear();
    platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &platformDevices);
    devices.insert(devices.end(), platformDevices.begin(), platformDevices.end());
  }
}
void CLHelpers::getContextDevices(const cl::Context& context, std::vector<cl::Device>& devices)
{
  devices.clear();

  if (context.getInfo<VECTOR_CLASS<cl::Device> >(CL_CONTEXT_DEVICES, &devices) != CL_SUCCESS)
  {
    fprintf(stderr, "getContextDevices(): Impossible d'accéder aux devices du contexte spécifié\r\n");
    exit(EXIT_FAILURE);
  }
}

void CLHelpers::printDeviceInfo(const cl::Device& device)
{
  std::string deviceName;
  cl_device_type deviceType;
  std::string deviceVersion;

  unsigned int deviceMaxComputeUnits;		// Nombre maximum d'unités de calculs parallèles (1 workgroup s'exécute sur une unité de calculs)
  unsigned int deviceMaxWorkItemDimensions;	// Nombre maximum de dimensions des workitems
  std::vector<size_t> deviceMaxWorkItemSizes;	// Nombre maximum de workitems assignable par dimension
  size_t deviceMaxWorkGroupSize;		// Nombre maximum de workitems p/ workgroup p/ unité de calcul

  // size_t deviceLocalMemSize;			// Taille de l'espace mémoire local en octets (mémoire commune aux workitems d'un workgroup)
  // size_t deviceGlobalMemSize;		// Taille de l'espace mémoire global en octets (mémoire commune à tous les workgroups)

  device.getInfo(CL_DEVICE_NAME, &deviceName);
  device.getInfo(CL_DEVICE_TYPE, &deviceType);
  device.getInfo(CL_DEVICE_VERSION, &deviceVersion);

  device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &deviceMaxComputeUnits);
  device.getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &deviceMaxWorkItemDimensions);
  device.getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &deviceMaxWorkItemSizes);
  device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &deviceMaxWorkGroupSize);

  // device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &deviceLocalMemSize);
  // device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &deviceGlobalMemSize);

  printf("Device name: %s\r\n", deviceName.c_str());
  printf("Device type: ");
  switch (deviceType)
  {
  case CL_DEVICE_TYPE_CPU: printf("CPU\r\n"); break;
  case CL_DEVICE_TYPE_GPU: printf("GPU\r\n"); break;
  case CL_DEVICE_TYPE_ACCELERATOR: printf("ACCELERATOR\r\n"); break;
  case CL_DEVICE_TYPE_DEFAULT: printf("DEFAULT\r\n"); break;
  default: printf("N/A\r\n"); break;
  }
  printf("Device version: %s\r\n", deviceVersion.c_str());

  printf("Device max compute units:%d\r\n", deviceMaxComputeUnits);
  printf("Device max workitem dimensions:%d\r\n", deviceMaxWorkItemDimensions);
  printf("Device max workitem sizes: [0]=%u, [1]=%u, [2]=%u\r\n", deviceMaxWorkItemSizes[0], deviceMaxWorkItemSizes[1], deviceMaxWorkItemSizes[2]);
  printf("Device max workgroup size:%u\r\n", deviceMaxWorkGroupSize);

  // printf("Device local memory size:%u octets\r\n", deviceLocalMemSize);
  // printf("Device global memory size:%u octets\r\n", deviceGlobalMemSize);
}
void CLHelpers::printDevicesInfo(const std::vector<cl::Device>& devices)
{
  unsigned int i;
  unsigned int count;

  count = devices.size();

  printf("\r\n");

  for (i = 0; i < count; ++i)
  {
    printf("--------------- Device [%d] ---------------\r\n", i);
    printf("\r\n");
    printDeviceInfo(devices[i]);
    printf("\r\n");
  }

  printf("\r\n");
}
void CLHelpers::printAllDeviceInfo()
{
  std::vector<cl::Device> devices;

  getAllDevices(devices);
  printDevicesInfo(devices);
}

void CLHelpers::printKernelInfo(const cl::Kernel& kernel, const cl::Device& device)
{
  //size_t kernelGlobalWorkSizes[3];
  size_t kernelWorkGroupSize;
  size_t kernelPreferredWorkGroupSizeMultiple;
  cl_ulong kernelPrivateMemSize;
  cl_ulong kernelLocalMemSize;

  //kernel.getWorkGroupInfo(device, CL_KERNEL_GLOBAL_WORK_SIZE, kernelGlobalWorkSizes);
  kernel.getWorkGroupInfo(device, CL_KERNEL_WORK_GROUP_SIZE, &kernelWorkGroupSize);
  kernel.getWorkGroupInfo(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &kernelPreferredWorkGroupSizeMultiple);
  kernel.getWorkGroupInfo(device, CL_KERNEL_PRIVATE_MEM_SIZE, &kernelPrivateMemSize);
  kernel.getWorkGroupInfo(device, CL_KERNEL_LOCAL_MEM_SIZE, &kernelLocalMemSize);

  //printf("Kernel global work size: [0] = %d, [1] = %d, [2] = %d\r\n", kernelGlobalWorkSizes[0], kernelGlobalWorkSizes[1], kernelGlobalWorkSizes[2]);
  printf("Kernel work group size: %u\r\n", kernelWorkGroupSize);
  printf("Kernel preferred work group size multiple: %u\r\n", kernelPreferredWorkGroupSizeMultiple);
  printf("Kernel private memory size: %llu bytes\r\n", kernelPrivateMemSize);
  printf("Kernel local memory size: %llu bytes\r\n", kernelLocalMemSize);
}

void CLHelpers::printPlatformInfo(const cl::Platform& platform)
{
  std::string platformName;
  std::string platformVendor;
  std::string platformProfile;
  std::string platformVersion;
  std::string platformExtensions;

  platform.getInfo(CL_PLATFORM_NAME, &platformName);
  platform.getInfo(CL_PLATFORM_VENDOR, &platformVendor);
  platform.getInfo(CL_PLATFORM_PROFILE, &platformProfile);
  platform.getInfo(CL_PLATFORM_VERSION, &platformVersion);
  platform.getInfo(CL_PLATFORM_EXTENSIONS, &platformExtensions);

  fprintf(stderr, "Name: %s\r\n", platformName.c_str());
  fprintf(stderr, "Vendor: %s\r\n", platformVendor.c_str());
  fprintf(stderr, "Profile: %s\r\n", platformProfile.c_str());
  fprintf(stderr, "Version: %s\r\n", platformVersion.c_str());
  fprintf(stderr, "Extensions: %s\r\n", platformExtensions.c_str());
}
void CLHelpers::printPlatformsInfo(const std::vector<cl::Platform>& platforms)
{
  uint i;
  uint count;

  count = platforms.size();

  printf("\r\n");

  for (i = 0; i < count; ++i)
  {
    printf("-------------------- Platform[%d] --------------------\r\n", i);
    printf("\r\n");

    printPlatformInfo(platforms[i]);

    printf("\r\n");
  }

  printf("------------------------------------------------------\r\n");
  printf("\r\n");
}
