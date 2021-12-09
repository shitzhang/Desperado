# Desperado  
This is my personal project for real-time ray tracing.  
The goal of the project is to denoise Monte-Carlo path traced images with low sample counts.

## Methods  
 - SVGF (Spatiotemporal Variance Guided Filtering)

### Features
 - Using OpenGL to generate primary visibility and to implement SVGF. Secondary and shadow rays are traced using OptiX.
 - Separation of albedo, direct lighting, and indirect lighting channels.
 - Spatial wavelet filtering -- taking into account normals, color edges, and depth/world-position differences.
 - Temporal accumulation -- Accumulation of color from previous frames, and variance estimation (to improve edge blurring).
 - Motion vectors -- reprojection to match up points across frames, even w/ movement.

### No Denoising
![1SPP (Sponza)](image/NoDenoising.png)

### Denoising - SVGF
![SVGF (Sponza)](image/SVGF.png)

### TODO
 - Add TAA in the final stage of rendering.
 - Find a Tone mapping method.

## Prerequisites
  - Visual Studio 2019
  - Nvidia Optix 6.5
  - CUDA Toolkit 10.1

## Acknowledgments
 - [1] [Spatiotemporal Variance-Guided Filtering: Real-Time Reconstruction for Path-Traced Global Illumination](https://research.nvidia.com/publication/2017-07_Spatiotemporal-Variance-Guided-Filtering%3A)
 - [2] [Edge-avoiding À-Trous wavelet transform for fast global illumination filtering](https://dl.acm.org/citation.cfm?id=1921491)

