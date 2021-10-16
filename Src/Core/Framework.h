
#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

// Define DLL export/import
#ifdef _MSC_VER
#define desperadoexport __declspec(dllexport)
#define desperadoimport __declspec(dllimport)
#elif defined(__GNUC__) // _MSC_VER
#define desperadoexport __attribute__ ((visibility ("default")))
#define desperadoimport extern
#endif // _MSC_VER

#ifdef DESPERADO_DLL
#define dlldecl desperadoexport
#else   // BUILDING_SHARED_DLL
#define dlldecl desperadoimport
#endif // BUILDING_SHARED_DLL

#include "Core/DesperadoConfig.h"
#include <stdint.h>
#include <memory>
#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <algorithm>
//#include "Utils/Logger.h"
#include "Utils/Math/Vector.h"

#ifndef arraysize
#define arraysize(a) (sizeof(a)/sizeof(a[0]))
#endif
#ifndef offsetof
#define offsetof(s, m) (size_t)( (ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)) )
#endif

#ifdef assert
#undef assert
#endif

#ifdef _DEBUG
#define assert(a)\
    if (!(a)) {\
        std::string str = "assertion failed(" + std::string(#a) + ")\nFile " + __FILE__ + ", line " + std::to_string(__LINE__);\
        printf("%s\n",str.c_str());\
    }

#define should_not_get_here() assert(false);

#else // _DEBUG

#ifdef _AUTOTESTING
#define assert(a) if (!(a)) throw std::exception("Assertion Failure");
#else // _AUTOTESTING
#define assert(a) ((void)(a))
#endif // _AUTOTESTING

#ifdef _MSC_VER
#define should_not_get_here() __assume(0)
#else // _MSC_VER
#define should_not_get_here() __builtin_unreachable()
#endif // _MSC_VER

#endif // _DEBUG

#define safe_delete(_a) {delete _a; _a = nullptr;}
#define safe_delete_array(_a) {delete[] _a; _a = nullptr;}
#define stringize(a) #a
#define concat_strings_(a, b) a##b
#define concat_strings(a, b) concat_strings_(a, b)

namespace Desperado
{
#define enum_class_operators(e_) \
    inline e_ operator& (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b)); } \
    inline e_ operator| (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b)); } \
    inline e_& operator|= (e_& a, e_ b) { a = a | b; return a; }; \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; }; \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a)); } \
    inline bool is_set(e_ val, e_ flag) { return (val & flag) != static_cast<e_>(0); } \
    inline void flip_bit(e_& val, e_ flag) { val = is_set(val, flag) ? (val & (~flag)) : (val | flag); }

    enum class ShaderType
    {
        Vertex,         ///< Vertex shader
        Pixel,          ///< Pixel shader
        Geometry,       ///< Geometry shader
        Hull,           ///< Hull shader (AKA Tessellation control shader)
        Domain,         ///< Domain shader (AKA Tessellation evaluation shader)
        Compute,        ///< Compute shader

        Count           ///< Shader Type count
    };


    /** Shading languages. Used for shader cross-compilation.
    */
    enum class ShadingLanguage
    {
        Unknown,        ///< Unknown language (e.g., for a plain .h file)
        GLSL,           ///< OpenGL Shading Language (GLSL)
        VulkanGLSL,     ///< GLSL for Vulkan
        HLSL,           ///< High-Level Shading Language
    };

    /** Framebuffer target flags. Used for clears and copy operations
    */
    enum class FboAttachmentType
    {
        None    = 0,    ///< Nothing. Here just for completeness
        Color   = 1,    ///< Operate on the color buffer.
        Depth   = 2,    ///< Operate on the the depth buffer.
        Stencil = 4,    ///< Operate on the the stencil buffer.

        All = Color | Depth | Stencil ///< Operate on all targets
    };

    enum_class_operators(FboAttachmentType);


    enum class ComparisonFunc
    {
        Disabled,       ///< Comparison is disabled
        Never,          ///< Comparison always fails
        Always,         ///< Comparison always succeeds
        Less,           ///< Passes if source is less than the destination
        Equal,          ///< Passes if source is equal to the destination
        NotEqual,       ///< Passes if source is not equal to the destination
        LessEqual,      ///< Passes if source is less than or equal to the destination
        Greater,        ///< Passes if source is greater than to the destination
        GreaterEqual,   ///< Passes if source is greater than or equal to the destination
    };

    /** Flags indicating what hot-reloadable resources have changed
    */
    enum class HotReloadFlags
    {
        None    = 0,    ///< Nothing. Here just for completeness
        Program = 1,    ///< Programs (shaders)
    };

    enum_class_operators(HotReloadFlags);

    /** Clamps a value within a range.
        \param[in] val Value to clamp
        \param[in] minVal Low end to clamp to
        \param[in] maxVal High end to clamp to
        \return Result
    */
    template<typename T>
    inline T clamp(const T& val, const T& minVal, const T& maxVal)
    {
        return std::min(std::max(val, minVal), maxVal);
    }

    /** Returns whether an integer number is a power of two.
    */
    template<typename T>
    inline typename std::enable_if<std::is_integral<T>::value, bool>::type isPowerOf2(T a)
    {
        return (a & (a - (T)1)) == 0;
    }

    template <typename T>
    inline T div_round_up(T a, T b) { return (a + b - (T)1) / b; }

#define align_to(_alignment, _val) ((((_val) + (_alignment) - 1) / (_alignment)) * (_alignment))

    /** Helper class to check if a class has a vtable.
        Usage: has_vtable<MyClass>::value is true if vtable exists, false otherwise.
    */
    template<class T>
    struct has_vtable
    {
        class derived : public T
        {
            virtual void force_the_vtable() {}
        };
        enum { value = (sizeof(T) == sizeof(derived)) };
    };

    /*! @} */
}

namespace Desperado
{
    /** Converts ShaderType enum elements to a string.
        \param[in] type Type to convert to string
        \return Shader type as a string
    */
    inline const std::string to_string(ShaderType Type)
    {
        switch(Type)
        {
        case ShaderType::Vertex:
            return "vertex";
        case ShaderType::Pixel:
            return "pixel";
        case ShaderType::Hull:
            return "hull";
        case ShaderType::Domain:
            return "domain";
        case ShaderType::Geometry:
            return "geometry";
        case ShaderType::Compute:
            return "compute";
        default:
            should_not_get_here();
            return "";
        }
    }
}

