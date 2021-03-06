#pragma once


#define CUDA_DIR "..\\..\\..\\Cuda"

#define SAMPLES_ABSOLUTE_INCLUDE_DIRS \
  "..\\..\\..\\third\\OptiX\\include", \
  "..\\..\\..\\third\\OptiX\\include\\optixu", \
  "C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v10.1\\include", 

// Signal whether to use NVRTC or not
#define CUDA_NVRTC_ENABLED 1

// NVRTC compiler options
#define CUDA_NVRTC_OPTIONS  \
  "-arch", \
  "compute_30", \
  "-use_fast_math", \
  "-lineinfo", \
  "-default-device", \
  "-rdc", \
  "true", \
  "-D__x86_64", \
  0,
