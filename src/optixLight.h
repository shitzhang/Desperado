
#pragma once
//#include <glm/glm.hpp>

struct OptixPointLight {
	optix::float3 position;
	optix::float3 emission;
};

struct OptixDirectionalLight {
	optix::float3 direction;
	optix::float3 emission;
};

struct OptixParallelogramLight {
	optix::float3 corner;
	optix::float3 v1, v2;
	optix::float3 normal;
	optix::float3 emission;
};
