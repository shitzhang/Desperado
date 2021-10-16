
#include <optix_world.h>
#include "common.h"
#include "helpers.h"

using namespace optix;

struct PerRayData_radiance
{
  float3 result;
  float  importance;
  int    depth;
};

rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(float,         scene_epsilon, , );
rtBuffer<float4, 2>              output_buffer;
rtDeclareVariable(rtObject,      top_object, , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );



RT_PROGRAM void pinhole_camera()
{

  float2 d = make_float2(launch_index) / make_float2(launch_dim) * 2.f - 1.f;
  float3 ray_origin = eye;
  float3 ray_direction = normalize(d.x*U + d.y*V + W);
  
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, RADIANCE_RAY_TYPE, scene_epsilon, RT_DEFAULT_MAX);

  PerRayData_radiance prd;
  prd.importance = 1.f;
  prd.depth = 0;

  rtTrace(top_object, ray, prd ) ;

  output_buffer[launch_index] = make_float4( prd.result , 1.0f );
  
}

RT_PROGRAM void exception()
{
  rtPrintExceptionDetails();
  output_buffer[launch_index] = make_float4( bad_color , 1.0f );
}



rtDeclareVariable(float3, bg_color, , );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

RT_PROGRAM void miss()
{
	prd_radiance.result = bg_color;
}
