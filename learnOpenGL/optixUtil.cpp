
#include "optixUtil.h"
#include "optixConfig.h"

#include <optixu/optixu_math_namespace.h>

#include <nvrtc.h>

#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <sstream>
#include <map>
#include <memory>

#if defined(_WIN32)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN 1
#    endif
#    include<windows.h>
#    include<mmsystem.h>
#else // Apple and Linux both use this
#    include<sys/time.h>
#    include <unistd.h>
#    include <dirent.h>
#endif



using namespace optix;

optix::Buffer createBufferImpl(
    optix::Context context,
    RTformat format,
    unsigned width,
    unsigned height,
    bool use_pbo,
    RTbuffertype buffer_type);

void optixUtil::reportErrorMessage(const char* message)
{
    std::cerr << "OptiX Error: '" << message << "'\n";
#if defined(_WIN32) && defined(RELEASE_PUBLIC)
    {
        char s[2048];
        sprintf(s, "OptiX Error: %s", message);
        MessageBoxA(0, s, "OptiX Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
    }
#endif
}


void optixUtil::handleError(RTcontext context, RTresult code, const char* file,
    int line)
{
    const char* message;
    char s[2048];
    rtContextGetErrorString(context, code, &message);
    sprintf(s, "%s\n(%s:%d)", message, file, line);
    reportErrorMessage(s);
}




void optixUtil::parseDimensions(const char* arg, int& width, int& height)
{

    // look for an 'x': <width>x<height>
    size_t width_end = strchr(arg, 'x') - arg;
    size_t height_begin = width_end + 1;

    if (height_begin < strlen(arg))
    {
        // find the beginning of the height string/
        const char* height_arg = &arg[height_begin];

        // copy width to null-terminated string
        char width_arg[32];
        strncpy(width_arg, arg, width_end);
        width_arg[width_end] = '\0';

        // terminate the width string
        width_arg[width_end] = '\0';

        width = atoi(width_arg);
        height = atoi(height_arg);
        return;
    }

    throw Exception(
        "Failed to parse width, heigh from string '" +
        std::string(arg) +
        "'");
}


double optixUtil::currentTime()
{
#if defined(_WIN32)

    // inv_freq is 1 over the number of ticks per second.
    static double inv_freq;
    static bool freq_initialized = false;
    static BOOL use_high_res_timer = 0;

    if (!freq_initialized)
    {
        LARGE_INTEGER freq;
        use_high_res_timer = QueryPerformanceFrequency(&freq);
        inv_freq = 1.0 / freq.QuadPart;
        freq_initialized = true;
    }

    if (use_high_res_timer)
    {
        LARGE_INTEGER c_time;
        if (QueryPerformanceCounter(&c_time))
            return c_time.QuadPart * inv_freq;
        else
            throw Exception("optixUtil::currentTime: QueryPerformanceCounter failed");
    }

    return static_cast<double>(timeGetTime()) * 1.0e-3;

#else

    struct timeval tv;
    if (gettimeofday(&tv, 0))
        throw Exception("optixUtil::urrentTime(): gettimeofday failed!\n");

    return  tv.tv_sec + tv.tv_usec * 1.0e-6;

#endif
}


void optixUtil::sleep(int seconds)
{
#if defined(_WIN32)
    Sleep(seconds * 1000);
#else
    ::sleep(seconds);
#endif
}


#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x
#define LINE_STR STRINGIFY(__LINE__)

// Error check/report helper for users of the C API
#define NVRTC_CHECK_ERROR( func )                                  \
  do {                                                             \
    nvrtcResult code = func;                                       \
    if( code != NVRTC_SUCCESS )                                    \
      throw Exception( "ERROR: " __FILE__ "(" LINE_STR "): " +     \
          std::string( nvrtcGetErrorString( code ) ) );            \
  } while( 0 )

static bool readSourceFile(std::string& str, const std::string& filename)
{
    // Try to open file
    std::ifstream file(filename.c_str());
    if (file.good())
    {
        // Found usable source file
        std::stringstream source_buffer;
        source_buffer << file.rdbuf();
        str = source_buffer.str();
        return true;
    }
    return false;
}



#if CUDA_NVRTC_ENABLED

static void getCuStringFromFile(std::string& cu, std::string& location, const char* sample_name, const char* filename)
{
    std::vector<std::string> source_locations;

    std::string base_dir = std::string(CUDA_DIR);

    // Potential source locations (in priority order)
    if (sample_name)
        source_locations.push_back(base_dir + "/" + sample_name + "/" + filename);
    else {
        source_locations.push_back(base_dir + "/" + filename);
    }

    for (std::vector<std::string>::const_iterator it = source_locations.begin(); it != source_locations.end(); ++it) {
        // Try to get source code from file
        if (readSourceFile(cu, *it))
        {
            location = *it;
            return;
        }

    }

    // Wasn't able to find or open the requested file
    throw Exception("Couldn't open source file " + std::string(filename));
}

static std::string g_nvrtcLog;

static void getPtxFromCuString(std::string& ptx, const char* sample_name, const char* cu_source, const char* name, const char** log_string)
{
    // Create program
    nvrtcProgram prog = 0;
    NVRTC_CHECK_ERROR(nvrtcCreateProgram(&prog, cu_source, name, 0, NULL, NULL));

    // Gather NVRTC options
    std::vector<const char*> options;

    std::string base_dir = std::string(CUDA_DIR);

    // Set sample dir as the primary include path
    std::string sample_dir;
    if (sample_name)
    {
        sample_dir = std::string("-I") + base_dir + "/" + sample_name;
        options.push_back(sample_dir.c_str());
    }

    // Collect include dirs
    std::vector<std::string> include_dirs;
    const char* abs_dirs[] = { SAMPLES_ABSOLUTE_INCLUDE_DIRS };
    //const char* rel_dirs[] = { SAMPLES_RELATIVE_INCLUDE_DIRS };

    const size_t n_abs_dirs = sizeof(abs_dirs) / sizeof(abs_dirs[0]);
    for (size_t i = 0; i < n_abs_dirs; i++)
        include_dirs.push_back(std::string("-I") + abs_dirs[i]);
    //const size_t n_rel_dirs = sizeof(rel_dirs) / sizeof(rel_dirs[0]);
    //for (size_t i = 0; i < n_rel_dirs; i++)
        //include_dirs.push_back(std::string("-I") + base_dir + rel_dirs[i]);
    for (std::vector<std::string>::const_iterator it = include_dirs.begin(); it != include_dirs.end(); ++it)
        options.push_back(it->c_str());

    // Collect NVRTC options
    const char* compiler_options[] = { CUDA_NVRTC_OPTIONS };
    const size_t n_compiler_options = sizeof(compiler_options) / sizeof(compiler_options[0]);
    for (size_t i = 0; i < n_compiler_options - 1; i++)
        options.push_back(compiler_options[i]);

    // JIT compile CU to PTX
    const nvrtcResult compileRes = nvrtcCompileProgram(prog, (int)options.size(), options.data());

    // Retrieve log output
    size_t log_size = 0;
    NVRTC_CHECK_ERROR(nvrtcGetProgramLogSize(prog, &log_size));
    g_nvrtcLog.resize(log_size);
    if (log_size > 1)
    {
        NVRTC_CHECK_ERROR(nvrtcGetProgramLog(prog, &g_nvrtcLog[0]));
        if (log_string)
            *log_string = g_nvrtcLog.c_str();
    }
    if (compileRes != NVRTC_SUCCESS)
        throw Exception("NVRTC Compilation failed.\n" + g_nvrtcLog);

    // Retrieve PTX code
    size_t ptx_size = 0;
    NVRTC_CHECK_ERROR(nvrtcGetPTXSize(prog, &ptx_size));
    ptx.resize(ptx_size);
    NVRTC_CHECK_ERROR(nvrtcGetPTX(prog, &ptx[0]));

    // Cleanup
    NVRTC_CHECK_ERROR(nvrtcDestroyProgram(&prog));
}

#else // CUDA_NVRTC_ENABLED

static void getPtxStringFromFile(std::string& ptx, const char* sample_name, const char* filename)
{
    std::string source_filename;
    if (sample_name)
        source_filename = std::string(optixUtil::samplesPTXDir()) + "/" + std::string(sample_name) + "_generated_" + std::string(filename) + ".ptx";
    else
        source_filename = std::string(optixUtil::samplesPTXDir()) + "/cuda_compile_ptx_generated_" + std::string(filename) + ".ptx";

    // Try to open source PTX file
    if (!readSourceFile(ptx, source_filename))
        throw Exception("Couldn't open source file " + source_filename);
}

#endif // CUDA_NVRTC_ENABLED

struct PtxSourceCache
{
    std::map<std::string, std::string*> map;
    ~PtxSourceCache()
    {
        for (std::map<std::string, std::string*>::const_iterator it = map.begin(); it != map.end(); ++it)
            delete it->second;
    }
};
static PtxSourceCache g_ptxSourceCache;

const char* optixUtil::getPtxString(
    const char* sample,
    const char* filename,
    const char** log)
{
    if (log)
        *log = NULL;

    std::string* ptx, cu;
    std::string key = std::string(filename) + ";" + (sample ? sample : "");
    std::map<std::string, std::string*>::iterator elem = g_ptxSourceCache.map.find(key);

    if (elem == g_ptxSourceCache.map.end())
    {
        ptx = new std::string();
#if CUDA_NVRTC_ENABLED
        std::string location;
        getCuStringFromFile(cu, location, sample, filename);
        getPtxFromCuString(*ptx, sample, cu.c_str(), location.c_str(), log);
#else
        getPtxStringFromFile(*ptx, sample, filename);
#endif
        g_ptxSourceCache.map[key] = ptx;
    }
    else
    {
        ptx = elem->second;
    }

    return ptx->c_str();
}

void optixUtil::ensureMinimumSize(int& w, int& h)
{
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
}

void optixUtil::ensureMinimumSize(unsigned& w, unsigned& h)
{
    if (w == 0) w = 1;
    if (h == 0) h = 1;
}

optix::Buffer optixUtil::createOutputBuffer(
    optix::Context context,
    RTformat format,
    unsigned width,
    unsigned height,
    bool use_pbo)
{
    return createBufferImpl(context, format, width, height, use_pbo, RT_BUFFER_OUTPUT);
}

optix::Buffer createBufferImpl(
    optix::Context context,
    RTformat format,
    unsigned width,
    unsigned height,
    bool use_pbo,
    RTbuffertype buffer_type)
{
    optix::Buffer buffer;
    if (use_pbo)
    {
        // First allocate the memory for the GL buffer, then attach it to OptiX.

        // Assume ubyte4 or float4 for now
        unsigned int elmt_size = format == RT_FORMAT_UNSIGNED_BYTE4 ? 4 : 16;

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, elmt_size * width * height, 0, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        buffer = context->createBufferFromGLBO(buffer_type, vbo);
        buffer->setFormat(format);
        buffer->setSize(width, height);
    }
    else
    {
        buffer = context->createBuffer(buffer_type, format, width, height);
    }

    return buffer;
}

void optixUtil::calculateCameraVariables(float3 eye, float3 lookat, float3 up,
    float  fov, float  aspect_ratio,
    float3& U, float3& V, float3& W, bool fov_is_vertical)
{
    float ulen, vlen, wlen;
    W = lookat - eye; // Do not normalize W -- it implies focal length

    wlen = length(W);
    U = normalize(cross(W, up));
    V = normalize(cross(U, W));

    if (fov_is_vertical) {
        vlen = wlen * tanf(0.5f * fov * M_PIf / 180.0f);
        V *= vlen;
        ulen = vlen * aspect_ratio;
        U *= ulen;
    }
    else {
        ulen = wlen * tanf(0.5f * fov * M_PIf / 180.0f);
        U *= ulen;
        vlen = ulen / aspect_ratio;
        V *= vlen;
    }
}

void optixUtil::displayBufferGL(optix::Buffer buffer,unsigned int &screen_tex_id)       
{
    // Query buffer information
    RTsize buffer_width_rts, buffer_height_rts;
    buffer->getSize(buffer_width_rts, buffer_height_rts);
    uint32_t width = static_cast<int>(buffer_width_rts);
    uint32_t height = static_cast<int>(buffer_height_rts);
    RTformat buffer_format = buffer->getFormat();


    //static unsigned int gl_tex_id = 0;
    if (!screen_tex_id)
    {
        glGenTextures(1, &screen_tex_id);
        glBindTexture(GL_TEXTURE_2D, screen_tex_id);

        // Change these to GL_LINEAR for super- or sub-sampling
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glBindTexture(GL_TEXTURE_2D, screen_tex_id);

    // send PBO or host-mapped image data to texture
    const unsigned pboId = buffer->getGLBOId();
    GLvoid* imageData = 0;
    if (pboId)
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
    else
        imageData = buffer->map(0, RT_BUFFER_MAP_READ);

    //RTsize elmt_size = buffer->getElementSize();
    //if (elmt_size % 8 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    //else if (elmt_size % 4 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    //else if (elmt_size % 2 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    //else                          glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //GLenum pixel_format = glFormatFromBufferFormat(g_image_buffer_format, buffer_format);

    //if (buffer_format == RT_FORMAT_UNSIGNED_BYTE4)
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, imageData);
    //else if (buffer_format == RT_FORMAT_FLOAT4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, imageData);
    //else if (buffer_format == RT_FORMAT_FLOAT3)
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, pixel_format, GL_FLOAT, imageData);
    //else if (buffer_format == RT_FORMAT_FLOAT)
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_FLOAT, width, height, 0, pixel_format, GL_FLOAT, imageData);
    //else
        //throw Exception("Unknown buffer format");

    if (pboId)
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    else
        buffer->unmap();
}

static const float FPS_UPDATE_INTERVAL = 0.5;  //seconds


void optixUtil::displayFps(unsigned int frame_count)
{
    static double fps = -1.0;
    static unsigned last_frame_count = 0;
    static double last_update_time = optixUtil::currentTime();
    static double current_time = 0.0;
    current_time = optixUtil::currentTime();
    if (current_time - last_update_time > FPS_UPDATE_INTERVAL) {
        fps = (frame_count - last_frame_count) / (current_time - last_update_time);
        last_frame_count = frame_count;
        last_update_time = current_time;
    }
    if (frame_count > 0 && fps >= 0.0) {
        printf("FPS: %7.2f\n", fps);
        //static char fps_text[32];
        //sprintf(fps_text, "fps: %7.2f", fps);
        //drawText(fps_text, 10.0f, 10.0f, GLUT_BITMAP_8_BY_13);
    }
}