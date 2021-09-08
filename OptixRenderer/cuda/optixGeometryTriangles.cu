/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <optix.h>
#include <optixu/optixu_math_namespace.h>

struct Vertex {
    float3 position;
    float3 normal;
    float2 texCoord;
};


rtDeclareVariable( float3, shading_normal,   attribute shading_normal, );
rtDeclareVariable( float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable( float2, texcoord,         attribute texcoord, );
rtDeclareVariable( float2, barycentrics,     attribute barycentrics, );

//rtBuffer<float3> vertex_buffer;
//rtBuffer<float3> normal_buffer;
//rtBuffer<float2> texcoord_buffer;
rtBuffer<uint3,1>   index_buffer;

rtBuffer<Vertex,1> vertex_buffer;

RT_PROGRAM void triangle_attributes()
{
    const uint3   v_idx = index_buffer[rtGetPrimitiveIndex()];
    //const float3 v0    = vertex_buffer[v_idx.x];
    const Vertex v0 = vertex_buffer[v_idx.x];
    //const float3 v1    = vertex_buffer[v_idx.y];
    const Vertex v1 = vertex_buffer[v_idx.y];
    //const float3 v2    = vertex_buffer[v_idx.z];
    const Vertex v2 = vertex_buffer[v_idx.z];
    const float3 Ng    = optix::cross( v1.position - v0.position, v2.position - v0.position );

    geometric_normal = optix::normalize( Ng );

    barycentrics = rtGetTriangleBarycentrics();
    //texcoord = make_float3( barycentrics.x, barycentrics.y, 0.0f );

    shading_normal = v1.normal * barycentrics.x + v2.normal * barycentrics.y
            + v0.normal * ( 1.0f-barycentrics.x-barycentrics.y );
    
    const float2 t0 = v0.texCoord;
    const float2 t1 = v1.texCoord;
    const float2 t2 = v2.texCoord;

    texcoord = t1*barycentrics.x + t2*barycentrics.y + t0*(1.0f-barycentrics.x-barycentrics.y);
}
