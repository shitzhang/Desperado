

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "phong.h"

using namespace optix;


rtDeclareVariable(float3,       Kd, , );
rtTextureSampler<float4, 2> texture_diffuse1;

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(float2, texcoord, attribute texcoord, ); 

rtDeclareVariable(PerRayData_radiance, prd, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

RT_PROGRAM void any_hit_shadow()
{
	prd_shadow.attenuation = optix::make_float3(0.0f);
	rtTerminateRay();
}

RT_PROGRAM void any_hit() {
  if (tex2D(texture_diffuse1, texcoord.x, texcoord.y).w < 0.1f) {
    rtIgnoreIntersection();
  } 
}

RT_PROGRAM void closest_hit()
{
  float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );

  float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

  float3 Kd_val = make_float3(tex2D( texture_diffuse1, texcoord.x, texcoord.y ));
  //float3 Kd_val = make_float3(tex2D( texture_diffuse1, 0.5f, 0.5f ));
  prd.result = Kd_val;
  //printf("color %f %f %f   ",prd.result.x,prd.result.y,prd.result.z);
}


