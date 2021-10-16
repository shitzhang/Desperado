
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
