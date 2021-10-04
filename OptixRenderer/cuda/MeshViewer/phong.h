

#include <optix_world.h>
#include "common.h"
#include "helpers.h"

struct PerRayData_radiance
{
  float3 result;
  float importance;
  int depth;
};

struct PerRayData_shadow
{
  float3 attenuation;
};


rtDeclareVariable(int,               max_depth, , );
rtBuffer<BasicLight>                 lights;
rtDeclareVariable(float3,            ambient_light_color, , );
rtDeclareVariable(float,             scene_epsilon, , );
rtDeclareVariable(rtObject,          top_object, , );
rtDeclareVariable(rtObject,          top_shadower, , );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
// rtDeclareVariable(PerRayData_radiance, prd, rtPayload, );
// rtDeclareVariable(PerRayData_shadow,   prd_shadow, rtPayload, );

// static __device__ void phongShadowed()
// {
//   // this material is opaque, so it fully attenuates all shadow rays
//   prd_shadow.attenuation = optix::make_float3(0.0f);
//   rtTerminateRay();
// }

// static
// __device__ void phongShade( float3 p_Kd,
//                             float3 p_Ka,
//                             float3 p_Ks,
//                             float3 p_Kr,
//                             float  p_phong_exp, 
//                             float3 p_normal )
// {
//   float3 hit_point = ray.origin + t_hit * ray.direction;
  
//   // ambient contribution

//   float3 result = p_Ka * ambient_light_color;

//   // compute direct lighting
//   unsigned int num_lights = lights.size();
//   for(int i = 0; i < num_lights; ++i) {
//     BasicLight light = lights[i];
//     float Ldist = optix::length(light.pos - hit_point);
//     float3 L = optix::normalize(light.pos - hit_point);
//     float nDl = optix::dot( p_normal, L);

//     // cast shadow ray
//     float3 light_attenuation = make_float3(static_cast<float>( nDl > 0.0f ));
//     if ( nDl > 0.0f && light.casts_shadow ) {
//       PerRayData_shadow shadow_prd;
//       shadow_prd.attenuation = make_float3(1.0f);
//       optix::Ray shadow_ray = optix::make_Ray( hit_point, L, SHADOW_RAY_TYPE, scene_epsilon, Ldist );
//       rtTrace(top_shadower, shadow_ray, shadow_prd);
//       light_attenuation = shadow_prd.attenuation;
//     }

//     // If not completely shadowed, light the hit point
//     if( fmaxf(light_attenuation) > 0.0f ) {
//       float3 Lc = light.color * light_attenuation;

//       result += p_Kd * nDl * Lc;

//       float3 H = optix::normalize(L - ray.direction);
//       float nDh = optix::dot( p_normal, H );
//       if(nDh > 0) {
//         float power = pow(nDh, p_phong_exp);
//         result += p_Ks * power * Lc;
//       }
//     }
//   }

//   if( fmaxf( p_Kr ) > 0 ) {

//     // ray tree attenuation
//     PerRayData_radiance new_prd;             
//     new_prd.importance = prd.importance * optix::luminance( p_Kr );
//     new_prd.depth = prd.depth + 1;

//     // reflection ray
//     if( new_prd.importance >= 0.01f && new_prd.depth <= max_depth) {
//       float3 R = optix::reflect( ray.direction, p_normal );
//       optix::Ray refl_ray = optix::make_Ray( hit_point, R, RADIANCE_RAY_TYPE, scene_epsilon, RT_DEFAULT_MAX );
//       rtTrace(top_object, refl_ray, new_prd);
//       result += p_Kr * new_prd.result;
//     }
//   }
  
//   // pass the color back up the tree
//   prd.result = result;
// }
