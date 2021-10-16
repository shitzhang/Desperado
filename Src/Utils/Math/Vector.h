#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"

#include <string>

namespace Desperado
{
    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;

    using uint2 = glm::uvec2;
    using uint3 = glm::uvec3;
    using uint4 = glm::uvec4;

    using int2 = glm::ivec2;
    using int3 = glm::ivec3;
    using int4 = glm::ivec4;

    using bool2 = glm::bvec2;
    using bool3 = glm::bvec3;
    using bool4 = glm::bvec4;

    inline std::string to_string(const float2& v) { return "float2(" + std::to_string(v.x) + "," + std::to_string(v.y) + ")"; }
    inline std::string to_string(const float3& v) { return "float3(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")"; }
    inline std::string to_string(const float4& v) { return "float4(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w) + ")"; }

    inline std::string to_string(const uint2& v) { return "uint2(" + std::to_string(v.x) + "," + std::to_string(v.y) + ")"; }
    inline std::string to_string(const uint3& v) { return "uint3(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")"; }
    inline std::string to_string(const uint4& v) { return "uint4(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w) + ")"; }

    inline std::string to_string(const int2& v) { return "int2(" + std::to_string(v.x) + "," + std::to_string(v.y) + ")"; }
    inline std::string to_string(const int3& v) { return "int3(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")"; }
    inline std::string to_string(const int4& v) { return "int4(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w) + ")"; }

    inline std::string to_string(const bool2& v) { return "bool2(" + std::to_string(v.x) + "," + std::to_string(v.y) + ")"; }
    inline std::string to_string(const bool3& v) { return "bool3(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")"; }
    inline std::string to_string(const bool4& v) { return "bool4(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w) + ")"; }
}
