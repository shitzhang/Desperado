#include "optixSVGF.h"
#include "../common.h"
#include "../random.h"

using namespace optix;

struct PerRayData_pathtrace
{
    float3 result;
    float3 radiance;
    float3 attenuation;
    float3 origin;
    float3 direction;
    float3 direct_radiance;
    float3 indirect_radiance;
    unsigned int seed;
    int depth;
    int countEmitted;
    int done;
};

struct PerRayData_pathtrace_shadow
{
    bool inShadow;
};

// Scene wide variables
rtDeclareVariable(float,         scene_epsilon, , );
rtDeclareVariable(rtObject,      top_object, , );
rtDeclareVariable(uint2,         launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2,         launch_dim,   rtLaunchDim, );

rtDeclareVariable(PerRayData_pathtrace, current_prd, rtPayload, );



//-----------------------------------------------------------------------------
//
//  Camera program -- main ray tracing loop
//
//-----------------------------------------------------------------------------

//rtDeclareVariable(float,         focal_length, , );
rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(unsigned int,  frame_number, , );
rtDeclareVariable(unsigned int,  sqrt_num_samples, , );
rtDeclareVariable(unsigned int,  rr_begin_depth, , );

rtBuffer<float4, 2>              output_buffer;
rtBuffer<float4, 2>              output_direct_buffer;
rtBuffer<float4, 2>              output_indirect_buffer;

rtBuffer<ParallelogramLight>     lights;

rtBuffer<float4, 2>              direct_color_buffer;
rtBuffer<float4, 2>              indirect_color_buffer;
rtBuffer<float4, 2>              color_buffer;


// RT_PROGRAM void pathtrace_camera()
// {
//     size_t2 screen = output_buffer.size();

//     float2 inv_screen = 1.0f/make_float2(screen) * 2.f;
//     float2 pixel = (make_float2(launch_index)) * inv_screen - 1.f;

//     float2 jitter_scale = inv_screen / sqrt_num_samples;
//     unsigned int samples_per_pixel = sqrt_num_samples*sqrt_num_samples;

//     float3 result = make_float3(0.0f);
//     float3 result_direct = make_float3(0.0f);
//     float3 result_indirect = make_float3(0.0f);

//     unsigned int seed = tea<16>(screen.x*launch_index.y+launch_index.x, frame_number);
//     while(samples_per_pixel--)
//     {
//         //
//         // Sample pixel using jittering
//         //
//         unsigned int x = samples_per_pixel%sqrt_num_samples;
      
//         unsigned int y = samples_per_pixel/sqrt_num_samples;
//         //printf("samples %u xy %u %u\n\n", samples_per_pixel, x, y);
//         //printf("Hello from index %u, %u!\n", launch_index.x, launch_index.y);
//         //printf("jitter %f\n%f\n\n",x - rnd(seed), y - rnd(seed));
//         //float2 jitter = make_float2(x+rnd(seed), y+rnd(seed));
//         float2 jitter = make_float2(x+0.5, y+0.5);
        
//         float2 d = pixel + jitter*jitter_scale;
//         float3 ray_origin = eye;
//         float3 ray_direction = normalize(d.x*U + d.y*V + W);

//         // Initialze per-ray data
//         PerRayData_pathtrace prd;
//         prd.result = make_float3(0.f);
//         prd.attenuation = make_float3(1.f);
//         prd.countEmitted = true;
//         prd.done = false;
//         prd.seed = seed;
//         prd.depth = 0;

//         // Each iteration is a segment of the ray path.  The closest hit will
//         // return new segments to be traced here.
//         for(;;)
//         {
//             Ray ray = make_Ray(ray_origin, ray_direction, RADIANCE_RAY_TYPE, scene_epsilon, RT_DEFAULT_MAX);
//             rtTrace(top_object, ray, prd);
          
//             if (prd.depth == 0) {
//                 prd.direct_radiance = prd.radiance * prd.attenuation;
//                 //printf("radiance : %f %f %f   attenuation : %f %f %f\n", prd.radiance.x, prd.radiance.y, prd.radiance.z, prd.attenuation.x, prd.attenuation.y, prd.attenuation.z);
//             }

//             if(prd.done)
//             {
//                 // We have hit the background or a luminaire
//                 prd.result += prd.radiance * prd.attenuation;
//                 break;
//             }

//             // Russian roulette termination 
//             if(prd.depth >= rr_begin_depth)
//             {
//                 float pcont = fmaxf(prd.attenuation);
//                 if(rnd(prd.seed) >= pcont)
//                     break;
//                 prd.attenuation /= pcont;
//             }

//             prd.depth++;
//             prd.result += prd.radiance * prd.attenuation;

//             //printf("radiance : %f %f %f   attenuation : %f %f %f\n", prd.radiance.x, prd.radiance.y, prd.radiance.z, prd.attenuation.x, prd.attenuation.y, prd.attenuation.z);

//             // Update ray data for the next path segment
//             ray_origin = prd.origin;
//             ray_direction = prd.direction;
//         }

//         prd.indirect_radiance = prd.result - prd.direct_radiance;

//         result += prd.result;
//         result_direct += prd.direct_radiance;
//         result_indirect += prd.indirect_radiance;
//         seed = prd.seed;
//     }

//     //
//     // Update the output buffer
//     //
//     float3 pixel_color = result/(sqrt_num_samples*sqrt_num_samples);
//     float3 pixel_color_direct = result_direct / (sqrt_num_samples * sqrt_num_samples);
//     float3 pixel_color_indirect = result_indirect / (sqrt_num_samples * sqrt_num_samples);

//     direct_color_buffer[launch_index] = make_float4(pixel_color_direct, 1.0f);
//     indirect_color_buffer[launch_index] = make_float4(pixel_color_indirect, 1.0f);
//     color_buffer[launch_index] = make_float4(pixel_color, 1.0f);
//     //printf("%f\n", pixel_color);

//     /*if (frame_number > 1)
//     {
//         float a = 1.0f / (float)frame_number;
//         float3 old_color = make_float3(output_buffer[launch_index]);
//         float3 old_direct_color = make_float3(output_direct_buffer[launch_index]);
//         float3 old_indirect_color = make_float3(output_indirect_buffer[launch_index]);

//         output_buffer[launch_index] = make_float4( lerp( old_color, pixel_color, a ), 1.0f );
//         output_direct_buffer[launch_index] = make_float4(lerp(old_direct_color, pixel_color_direct, a), 1.0f);
//         output_indirect_buffer[launch_index] = make_float4(lerp(old_indirect_color, pixel_color_indirect, a), 1.0f);
//     }
//     else
//     {
//         output_buffer[launch_index] = make_float4(pixel_color, 1.0f);
//         output_direct_buffer[launch_index] = make_float4(pixel_color_direct, 1.0f);
//         output_indirect_buffer[launch_index] = make_float4(pixel_color_indirect, 1.0f);
//     }*/
// }

// rtBuffer<float3, 2>              world_pos_buffer;
// rtBuffer<float3, 2>              world_normal_buffer;
// rtBuffer<float3, 2>              albedo_buffer;

rtTextureSampler<float4, 2>              world_pos_tex;
rtTextureSampler<float4, 2>              world_normal_tex;
rtTextureSampler<float4, 2>              albedo_tex;

RT_PROGRAM void pathtrace_Gbuffer()
{
    size_t2 screen = launch_dim;
    unsigned int seed = tea<16>(screen.x * launch_index.y + launch_index.x, frame_number);

    float2 texCd = (make_float2(launch_index) + 0.5) / make_float2(screen);

    float3 ray_origin = make_float3(tex2D(world_pos_tex, texCd.x, texCd.y));
    float3 ffnormal = make_float3(tex2D(world_normal_tex, texCd.x, texCd.y));
    float3 diffuse_color = make_float3(tex2D(albedo_tex, texCd.x, texCd.y));

    float z1 = rnd(seed);
    float z2 = rnd(seed);
    float3 ray_direction;
    cosine_sample_hemisphere(z1, z2, ray_direction);
    optix::Onb onb( ffnormal );
    onb.inverse_transform( ray_direction );

    // Initialze per-ray data
    PerRayData_pathtrace prd;
    prd.result = make_float3(0.f);
    prd.attenuation = diffuse_color;
    prd.countEmitted = false;
    prd.done = false;
    prd.seed = seed;
    prd.depth = 0;

    unsigned int num_lights = lights.size();
    float3 lightRadiance = make_float3(0.0f);

    for(int i = 0; i < num_lights; ++i)
    {
        // Choose random point on light
        ParallelogramLight light = lights[i];
        const float z1 = rnd(prd.seed);
        const float z2 = rnd(prd.seed);
        const float3 light_pos = light.corner + light.v1 * z1 + light.v2 * z2;

        // Calculate properties of light sample (for area based pdf)
        const float  Ldist = length(light_pos - ray_origin);
        const float3 L     = normalize(light_pos - ray_origin);
        const float  nDl   = dot( ffnormal, L );
        const float  LnDl  = dot( light.normal, L );

        // cast shadow ray
        if ( nDl > 0.0f && LnDl > 0.0f )
        {
            PerRayData_pathtrace_shadow shadow_prd;
            shadow_prd.inShadow = false;
            // Note: bias both ends of the shadow ray, in case the light is also present as geometry in the scene.
            Ray shadow_ray = make_Ray( ray_origin, L, SHADOW_RAY_TYPE, scene_epsilon, Ldist - scene_epsilon );
            rtTrace(top_object, shadow_ray, shadow_prd);

            if(!shadow_prd.inShadow)
            {
                const float A = length(cross(light.v1, light.v2));
                // convert area based pdf to solid angle
                const float weight = nDl * LnDl * A / (M_PIf * Ldist * Ldist);
                lightRadiance += light.emission * weight;
            }
        }
    }
    prd.radiance = lightRadiance;
    prd.result = prd.radiance * prd.attenuation;

    prd.direct_radiance = prd.radiance * prd.attenuation;


    // Each iteration is a segment of the ray path.  The closest hit will
    // return new segments to be traced here.
    for (;;)
    {
        Ray ray = make_Ray(ray_origin, ray_direction, RADIANCE_RAY_TYPE, scene_epsilon, RT_DEFAULT_MAX);
        rtTrace(top_object, ray, prd);

        // if (prd.depth == 0) {
        //     prd.direct_radiance = prd.radiance * prd.attenuation;
        //     //printf("radiance : %f %f %f   attenuation : %f %f %f\n", prd.radiance.x, prd.radiance.y, prd.radiance.z, prd.attenuation.x, prd.attenuation.y, prd.attenuation.z);
        // }
        prd.depth++;
        if (prd.done)
        {
            // We have hit the background or a luminaire
            prd.result += prd.radiance * prd.attenuation;
            break;
        }

        // Russian roulette termination 
        if (prd.depth >= rr_begin_depth)
        {
            float pcont = fmaxf(prd.attenuation);
            if (rnd(prd.seed) >= pcont)
                break;
            prd.attenuation /= pcont;
        }

        //prd.depth++;
        prd.result += prd.radiance * prd.attenuation;

        //printf("radiance : %f %f %f   attenuation : %f %f %f\n", prd.radiance.x, prd.radiance.y, prd.radiance.z, prd.attenuation.x, prd.attenuation.y, prd.attenuation.z);

        // Update ray data for the next path segment
        ray_origin = prd.origin;
        ray_direction = prd.direction;
    }

    prd.indirect_radiance = prd.result - prd.direct_radiance;

    float3 result = prd.result;
    float3 result_direct = prd.direct_radiance;
    float3 result_indirect = prd.indirect_radiance;
    
    float3 pixel_color = result;
    float3 pixel_color_direct = result_direct;
    float3 pixel_color_indirect = result_indirect;

    direct_color_buffer[launch_index] = make_float4(pixel_color_direct, 1.0f);
    indirect_color_buffer[launch_index] = make_float4(pixel_color_indirect, 1.0f);
    color_buffer[launch_index] = make_float4(pixel_color, 1.0f);

    if (frame_number > 1)
    {
        float a = 1.0f / (float)frame_number;
        float3 old_color = make_float3(output_buffer[launch_index]);
        float3 old_direct_color = make_float3(output_direct_buffer[launch_index]);
        float3 old_indirect_color = make_float3(output_indirect_buffer[launch_index]);

        output_buffer[launch_index] = make_float4( lerp( old_color, pixel_color, a ), 1.0f );
        output_direct_buffer[launch_index] = make_float4(lerp(old_direct_color, pixel_color_direct, a), 1.0f);
        output_indirect_buffer[launch_index] = make_float4(lerp(old_indirect_color, pixel_color_indirect, a), 1.0f);
    }
    else
    {
        output_buffer[launch_index] = make_float4(pixel_color, 1.0f);
        output_direct_buffer[launch_index] = make_float4(pixel_color_direct, 1.0f);
        output_indirect_buffer[launch_index] = make_float4(pixel_color_indirect, 1.0f);
    }
}


rtDeclareVariable(float3,        emission_color, , );

RT_PROGRAM void closest_hit_emitter()
{
    current_prd.radiance = current_prd.countEmitted ? emission_color : make_float3(0.f);
    current_prd.done = true;
}


rtDeclareVariable(float3,       Kd, , );
rtTextureSampler<float4, 2>   diffuse_map1;

rtDeclareVariable(float3,     geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3,     shading_normal,   attribute shading_normal, );
rtDeclareVariable(float2,     texcoord,         attribute texcoord, ); 
rtDeclareVariable(uint,       mesh_id,          attribute mesh_id, );

rtDeclareVariable(optix::Ray, ray,              rtCurrentRay, );
rtDeclareVariable(float,      t_hit,            rtIntersectionDistance, );


RT_PROGRAM void closest_hit()
{
    float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
    float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
    float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

    float3 hitpoint = ray.origin + t_hit * ray.direction;

    //
    // Generate a reflection ray.  This will be traced back in ray-gen.
    //
    current_prd.origin = hitpoint;

    float z1=rnd(current_prd.seed);
    float z2=rnd(current_prd.seed);
    float3 p;
    cosine_sample_hemisphere(z1, z2, p);
    optix::Onb onb( ffnormal );
    onb.inverse_transform( p );
    current_prd.direction = p;

    // NOTE: f/pdf = 1 since we are perfectly importance sampling lambertian
    // with cosine density.
    float3 diffuse_color = make_float3(tex2D(diffuse_map1, texcoord.x, texcoord.y));
    //float3 diffuse_color = make_float3(1.0f);
    current_prd.attenuation = current_prd.attenuation * diffuse_color;
    current_prd.countEmitted = false;

    //
    // Next event estimation (compute direct lighting).
    //
    unsigned int num_lights = lights.size();
    float3 result = make_float3(0.0f);

    for(int i = 0; i < num_lights; ++i)
    {
        // Choose random point on light
        ParallelogramLight light = lights[i];
        const float z1 = rnd(current_prd.seed);
        const float z2 = rnd(current_prd.seed);
        const float3 light_pos = light.corner + light.v1 * z1 + light.v2 * z2;

        // Calculate properties of light sample (for area based pdf)
        const float  Ldist = length(light_pos - hitpoint);
        const float3 L     = normalize(light_pos - hitpoint);
        const float  nDl   = dot( ffnormal, L );
        const float  LnDl  = dot( light.normal, L );

        // cast shadow ray
        if ( nDl > 0.0f && LnDl > 0.0f )
        {
            PerRayData_pathtrace_shadow shadow_prd;
            shadow_prd.inShadow = false;
            // Note: bias both ends of the shadow ray, in case the light is also present as geometry in the scene.
            Ray shadow_ray = make_Ray( hitpoint, L, SHADOW_RAY_TYPE, scene_epsilon, Ldist - scene_epsilon );
            //top_shadower is not used.
            rtTrace(top_object, shadow_ray, shadow_prd);

            if(!shadow_prd.inShadow)
            {
                const float A = length(cross(light.v1, light.v2));
                // convert area based pdf to solid angle
                const float weight = nDl * LnDl * A / (M_PIf * Ldist * Ldist);
                result += light.emission * weight;
            }
        }
    }

    current_prd.radiance = result;
}



rtDeclareVariable(PerRayData_pathtrace_shadow, current_prd_shadow, rtPayload, );

RT_PROGRAM void any_hit_shadow()
{
    current_prd_shadow.inShadow = true;
    rtTerminateRay();
}

RT_PROGRAM void any_hit()
{
    if (tex2D(diffuse_map1, texcoord.x, texcoord.y).w < 0.1f) {
        rtIgnoreIntersection();
    } 
}



RT_PROGRAM void exception()
{
    printf("optix: gg");
    output_buffer[launch_index] = make_float4(bad_color, 1.0f);
}



rtDeclareVariable(float3, bg_color, , );

RT_PROGRAM void miss()
{
    current_prd.radiance = bg_color;
    current_prd.done = true;
}


