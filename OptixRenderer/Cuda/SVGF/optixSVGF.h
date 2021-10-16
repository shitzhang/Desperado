
#pragma once

#include <optixu/optixu_math_namespace.h>  
#include <optixu/optixu_matrix_namespace.h>

struct ParallelogramLight                                                        
{                                                                                
    optix::float3 corner;                                                          
    optix::float3 v1, v2;                                                          
    optix::float3 normal;                                                          
    optix::float3 emission;                                                        
};

//static __device__ float2 calcMotionVector(float3 hitpoint,float4 screen_pos, optix::Matrix4x4 pre_view_mat, optix::Matrix4x4 pre_projection_mat)
//{
//
//    float4 pre_screen_pos = (pre_projection_mat * pre_view_mat* make_float4(hitpoint, 1.0));
//    pre_screen_pos /= pre_screen_pos.w;
//    float2 delta = make_float2(pre_screen_pos.x, pre_screen_pos.y) - make_float2(screen_pos.x, screen_pos.y);
//    float2 uv_delta = (delta + make_float2(1.0, 1.0)) * 0.5;
//    return uv_delta;
//}

