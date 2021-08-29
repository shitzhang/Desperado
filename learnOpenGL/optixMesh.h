#pragma once

#include "mesh.h"
#include "optixUtil.h"


#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h> 
//#include <optixu/optixu_math_stream_namespace.h>

struct MeshBuffers
{
	optix::Buffer vertices;
	optix::Buffer indices;
	//optix::Buffer tri_indices;
	//optix::Buffer mat_indices;
	//optix::Buffer positions;
	//optix::Buffer normals;
	//optix::Buffer texcoords;
};

class OptiXMesh
{
public:
	OptiXMesh(std::shared_ptr<Mesh> mesh,optix::Context context) :p_mesh(mesh),context(context)
	{
		init();
	}
	void init() {

		unsigned int num_triangles = p_mesh->indices.size() / 3;

		auto vbo = p_mesh->getVBO();
		auto ebo = p_mesh->getEBO();

		m_buffers.vertices = context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getVBO());
		m_buffers.vertices->setFormat(RT_FORMAT_USER);
		m_buffers.vertices->setElementSize(sizeof(Vertex));
		m_buffers.vertices->setSize(p_mesh->vertices.size());

		m_buffers.indices = context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getEBO());
		m_buffers.indices->setFormat(RT_FORMAT_UNSIGNED_INT3);
		m_buffers.indices->setSize(num_triangles);

		optix::GeometryTriangles geoTris = context->createGeometryTriangles();
		geoTris->setPrimitiveCount(num_triangles);

		geoTris->setVertices(p_mesh->vertices.size(), m_buffers.vertices, 0, sizeof(Vertex), RT_FORMAT_FLOAT3);
		geoTris->setTriangleIndices(m_buffers.indices, RT_FORMAT_UNSIGNED_INT3);
		geoTris->setBuildFlags(RTgeometrybuildflags(0));

		const char* ptx = optixUtil::getPtxString(0, "optixGeometryTriangles.cu");
		attribute = context->createProgramFromPTXString(ptx, "triangle_attributes");
		geoTris->setAttributeProgram(attribute);

		ptx = optixUtil::getPtxString(SAMPLE_NAME, "phong.cu");
		closest_hit = context->createProgramFromPTXString(ptx, "closest_hit_radiance");
		any_hit = context->createProgramFromPTXString(ptx, "any_hit_shadow");
		material->setClosestHitProgram(0, closest_hit);
		material->setAnyHitProgram(1, any_hit);
		glm::vec3 kd = p_mesh->mat.kd;
		material["kd"]->setFloat(optix::make_float3(kd.r, kd.g, kd.b));


		geoTris["index_buffer"]->setBuffer(m_buffers.indices);
		geoTris["vertex_buffer"]->setBuffer(m_buffers.vertices);



		geom_instance = context->createGeometryInstance();
		geom_instance->setGeometryTriangles(geoTris);

		//optix::GeometryGroup ggTris = context->createGeometryGroup();
		//ggTris->addChild(geom_instance);
		//ggTris->setAcceleration(context->createAcceleration("Trbvh"));

		//return ggTris;
	}

	optix::GeometryInstance getMeshGeometry() {
		return geom_instance;
	}

	std::shared_ptr<Mesh> p_mesh;

	optix::Context               context;       // required

private:
	MeshBuffers m_buffers;
	optix::Material              material;      // optional single matl override

	//optix::Program               intersection;  // optional 
	//optix::Program               bounds;        // optional
	optix::Program               attribute;        // optional

	optix::Program               closest_hit;   // optional multi matl override
	optix::Program               any_hit;       // optional


	// Output
	optix::GeometryInstance      geom_instance;
	//optix::float3                bbox_min;
	//optix::float3                bbox_max;

	unsigned int                 num_triangles;
};