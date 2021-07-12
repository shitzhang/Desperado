#pragma once


#define CUDA_DIR "F:/learnOpenGL/OptixRenderer/cuda"
#define PTX_DIR "F:/learnOpenGL/OptixRenderer/ptx"

 // Include directories
#define SAMPLES_RELATIVE_INCLUDE_DIRS \
  "/sutil", \
  "/cuda", 
#define SAMPLES_ABSOLUTE_INCLUDE_DIRS \
  "C:/ProgramData/NVIDIA Corporation/OptiX SDK 6.5.0/include", \
  "C:/ProgramData/NVIDIA Corporation/OptiX SDK 6.5.0/include/optixu", \
  "C:/ProgramData/NVIDIA Corporation/OptiX SDK 6.5.0/SDK/support/mdl-sdk/include", \
  "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.1/include", 

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
