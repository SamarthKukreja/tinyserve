#include <cuda_runtime.h>

#include <cstdlib>
#include <iostream>

int main() {
  int device_count = 0;
  const auto count_status = cudaGetDeviceCount(&device_count);
  if (count_status == cudaErrorNoDevice || device_count == 0) {
    std::cout << "TinyServe CUDA toolkit is available, but no CUDA device is visible\n";
    return 77;
  }
  if (count_status != cudaSuccess) {
    std::cerr << "cudaGetDeviceCount failed: " << cudaGetErrorString(count_status) << '\n';
    return EXIT_FAILURE;
  }

  int runtime_version = 0;
  int driver_version = 0;
  if (cudaRuntimeGetVersion(&runtime_version) != cudaSuccess ||
      cudaDriverGetVersion(&driver_version) != cudaSuccess) {
    std::cerr << "failed to query CUDA runtime or driver version\n";
    return EXIT_FAILURE;
  }
  std::cout << "cuda_device_count=" << device_count << '\n'
            << "cuda_runtime_version=" << runtime_version << '\n'
            << "cuda_driver_version=" << driver_version << '\n';
  for (int device = 0; device < device_count; ++device) {
    cudaDeviceProp properties{};
    const auto property_status = cudaGetDeviceProperties(&properties, device);
    if (property_status != cudaSuccess) {
      std::cerr << "cudaGetDeviceProperties failed: "
                << cudaGetErrorString(property_status) << '\n';
      return EXIT_FAILURE;
    }
    std::cout << "device=" << device << ",name=" << properties.name
              << ",compute_capability=" << properties.major << '.' << properties.minor
              << ",global_memory_bytes=" << properties.totalGlobalMem << '\n';
  }
  return EXIT_SUCCESS;
}
