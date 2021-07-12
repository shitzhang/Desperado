#pragma once
#pragma once

#include "global.h"

#include <optixu/optixpp_namespace.h>

#include <stdlib.h>
#include <vector>


// Default catch block
#define SUTIL_CATCH( ctx ) catch( const sutil::APIError& e ) {     \
    sutil::handleError( ctx, e.code, e.file.c_str(), e.line );     \
  }                                                                \
  catch( const std::exception& e ) {                               \
    sutil::reportErrorMessage( e.what() );                         \
    exit(1);                                                       \
  }

// Error check/report helper for users of the C API
#define RT_CHECK_ERROR( func )                                     \
  do {                                                             \
    RTresult code = func;                                          \
    if( code != RT_SUCCESS )                                       \
      throw sutil::APIError( code, __FILE__, __LINE__ );           \
  } while(0)



namespace optixUtil
{

	// Exeption to be thrown by RT_CHECK_ERROR macro
	struct APIError
	{
		APIError(RTresult c, const std::string& f, int l)
			: code(c), file(f), line(l) {}
		RTresult     code;
		std::string  file;
		int          line;
	};

	// Display error message
	void  reportErrorMessage(
		const char* message);               // Error mssg to be displayed

// Queries provided RTcontext for last error message, displays it, then exits.
	void  handleError(
		RTcontext context,                  // Context associated with the error
		RTresult code,                      // Code returned by OptiX API call
		const char* file,                   // Filename for error reporting
		int line);                          // File lineno for error reporting

// Query top level samples directory.
// The pointer returned may point to a static array.
	const char* samplesDir();

	// Query directory containing PTX files for compiled sample CUDA files.
	// The pointer returned may point to a static array.
	const char* samplesPTXDir();

	// Query directory containing CUDA files for NVRTC to compile at runtime.
	// The pointer returned may point to a static array.
	const char* samplesCUDADir();

	// Create an output buffer with given specifications
	optix::Buffer  createOutputBuffer(
		optix::Context context,             // optix context
		RTformat format,                    // Pixel format (must be ubyte4 for pbo)
		unsigned width,                     // Buffer width
		unsigned height,                    // Buffer height
		bool use_pbo);                      // Buffer type                    

// Create an input/output buffer with given specifications
	optix::Buffer  createInputOutputBuffer(
		optix::Context context,             // optix context
		RTformat format,                    // Pixel format (must be ubyte4 for pbo)
		unsigned width,                     // Buffer width
		unsigned height,                    // Buffer height
		bool use_pbo);                      // Buffer type                    

	// Resize a Buffer and its underlying GLBO if necessary
	void  resizeBuffer(
		optix::Buffer buffer,               // Buffer to be modified
		unsigned width,                     // New buffer width
		unsigned height);                  // New buffer height


// Create GLUT window and display contents of the buffer.
	void  displayBufferGlut(
		const char* window_title,           // Window title
		optix::Buffer buffer);              // Buffer to be displayed

// Create GLUT window and display contents of the buffer (C API version).
	void  displayBufferGlut(
		const char* window_title,           // Window title
		RTbuffer buffer);                   // Buffer to be displayed

// Write the contents of the Buffer to a PPM image file
	void  displayBufferPPM(
		const char* filename,                 // Image file to be created
		optix::Buffer buffer,                 // Buffer to be displayed
		bool disable_srgb_conversion = true); // Enables/disables srgb conversion before the image is saved. Disabled by default.            

// Write the contents of the Buffer to a PPM image file (C API version).
	void  displayBufferPPM(
		const char* filename,                 // Image file to be created
		RTbuffer buffer,                      // Buffer to be displayed
		bool disable_srgb_conversion = true); // Enables/disables srgb conversion before the image is saved. Disabled by default.            

// Display contents of buffer, where the OpenGL/GLUT context is managed by caller.
	void  displayBufferGL(
		optix::Buffer buffer,unsigned int &screen_tex_id);      // Buffer to be displayed
		 // The pixel format of the buffer or 0 to use the default for the pixel type
		

	// Display frames per second, where the OpenGL/GLUT context
	// is managed by the caller.
	void  displayFps(unsigned total_frame_count);

	// Display a short string starting at x,y.
	void  displayText(const char* text, float x, float y);

	// Create on OptiX TextureSampler for the given image file.  If the filename is
	// empty or if loading the file fails, return 1x1 texture with default color.
	optix::TextureSampler  loadTexture(
		optix::Context context,             // Context used for object creation 
		const std::string& filename,        // File to load
		optix::float3 default_color);       // Default color in case of file failure


// Create an OptiX Buffer for the given image file.  If the file load fails, 
// a null pointer is returned.
	optix::Buffer  loadPPMFloat4Buffer(optix::Context     context,     // Context used for object creation
		const std::string& filename);  // File to load


// Creates a Buffer object for the given cubemap files.
	optix::Buffer  loadCubeBuffer(
		optix::Context context,             // Context used for object creation
		const std::vector<std::string>& filenames); // Files to be loaded 


// Calculate appropriate U,V,W for pinhole_camera shader.
	void  calculateCameraVariables(
		optix::float3 eye,                  // Camera eye position
		optix::float3 lookat,               // Point in scene camera looks at
		optix::float3 up,                   // Up direction
		float  fov,                         // Horizontal or vertical field of view (assumed horizontal, see boolean below)
		float  aspect_ratio,                // Pixel aspect ratio (width/height)
		optix::float3& U,                   // [out] U coord for camera program
		optix::float3& V,                   // [out] V coord for camera program
		optix::float3& W,                   // [out] W coord for camera program
		bool fov_is_vertical = false);

	// Blocking sleep call
	void  sleep(
		int seconds);                      // Number of seconds to sleep

// Parse the string of the form <width>x<height> and return numeric values.
	void  parseDimensions(
		const char* arg,                    // String of form <width>x<height>
		int& width,                         // [out] width
		int& height);                      // [in]  height

// Get current time in seconds for benchmarking/timing purposes.
	double  currentTime();

	// Get PTX, either pre-compiled with NVCC or JIT compiled by NVRTC.
	const char* getPtxString(
		const char* sample,                 // Name of the sample, used to locate the input file. NULL = only search the common /cuda dir
		const char* filename,               // Cuda C input file name
		const char** log = NULL);          // (Optional) pointer to compiler log string. If *log == NULL there is no output. Only valid until the next getPtxString call

// Ensures that width and height have the minimum size to prevent launch errors.
	void  ensureMinimumSize(
		int& width,                             // Will be assigned the minimum suitable width if too small.
		int& height);                           // Will be assigned the minimum suitable height if too small.

	// Ensures that width and height have the minimum size to prevent launch errors.
	void  ensureMinimumSize(
		unsigned& width,                        // Will be assigned the minimum suitable width if too small.
		unsigned& height);                      // Will be assigned the minimum suitable height if too small.

}