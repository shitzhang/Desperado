#pragma once

#include "mesh.h"
#include "optixUtil.h"


#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h> 
//#include <optixu/optixu_math_stream_namespace.h>

#include <map>

namespace Desperado {
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

		static std::map<unsigned int, optix::TextureSampler> map_textureSampler;

		OptiXMesh(std::shared_ptr<Mesh> mesh, optix::Context context) :p_mesh(mesh), context(context)
		{
			init();
		}
		void init() {

			unsigned int num_triangles = p_mesh->indices.size() / 3;

			//for (auto const& v : p_mesh->vertices) {
			//	if (v.TexCoords.x < -10 || v.TexCoords.y < -10) {
			//		cout << "dd" << endl;
			//	}
			//}

			auto vbo = p_mesh->getVBO();
			auto ebo = p_mesh->getEBO();

			m_buffers.vertices = context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getVBO());
			m_buffers.vertices->setFormat(RT_FORMAT_USER);

			m_buffers.vertices->setElementSize(sizeof(Vertex));
			m_buffers.vertices->setSize(p_mesh->vertices.size());

			m_buffers.indices = context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getEBO());
			m_buffers.indices->setFormat(RT_FORMAT_UNSIGNED_INT3);
			m_buffers.indices->setSize(num_triangles);

			//optix::GeometryTriangles geoTris = context->createGeometryTriangles(); 
			//geoTris->setPrimitiveCount(num_triangles);

			//geoTris->setVertices(p_mesh->vertices.size(), m_buffers.vertices, 0, sizeof(float) * 8, RT_FORMAT_FLOAT3);
			//geoTris->setTriangleIndices(m_buffers.indices, RT_FORMAT_UNSIGNED_INT3);
			////geoTris->setBuildFlags(RTgeometrybuildflags(0));

			//geoTris["index_buffer"]->setBuffer(m_buffers.indices);
			//geoTris["vertex_buffer"]->setBuffer(m_buffers.vertices);

			//const char* ptx = optixUtil::getPtxString(0, "optixGeometryTriangles.cu");
			//attribute = context->createProgramFromPTXString(ptx, "triangle_attributes");
			//geoTris->setAttributeProgram(attribute);

			optix::Geometry g = context->createGeometry();
			g["vertex_buffer"]->setBuffer(m_buffers.vertices);
			g["index_buffer"]->setBuffer(m_buffers.indices);
			g->setPrimitiveCount(num_triangles);

			std::string ptx = optixUtil::getPtxString(0, "triangle_mesh.cu");
			intersection = context->createProgramFromPTXString(ptx, "mesh_intersect");
			bounds = context->createProgramFromPTXString(ptx, "mesh_bounds");

			g->setIntersectionProgram(intersection);
			g->setBoundingBoxProgram(bounds);

			/*ptx = optixUtil::getPtxString(SAMPLE_NAME, "phong.cu");
			closest_hit = context->createProgramFromPTXString(ptx, "closest_hit");
			any_hit = context->createProgramFromPTXString(ptx, "any_hit");
			any_hit_shadow = context->createProgramFromPTXString(ptx, "any_hit_shadow");

			material = context->createMaterial();
			material->setClosestHitProgram(0, closest_hit);
			material->setAnyHitProgram(0, any_hit);
			material->setAnyHitProgram(1, any_hit_shadow);*/

			material = context->createMaterial();
			ptx = optixUtil::getPtxString(SAMPLE_NAME, "optixPathTracer.cu");
			closest_hit = context->createProgramFromPTXString(ptx, "closest_hit");
			any_hit = context->createProgramFromPTXString(ptx, "any_hit");
			any_hit_shadow = context->createProgramFromPTXString(ptx, "any_hit_shadow");
			material->setClosestHitProgram(0, closest_hit);
			material->setAnyHitProgram(0, any_hit);
			material->setAnyHitProgram(1, any_hit_shadow);

			unsigned int diffuse_num = 1;
			//unsigned int specularNr = 1;
			//unsigned int normalNr = 1;
			//unsigned int heightNr = 1;

			if (p_mesh->textures.size() == 0) {
				cout << "mesh: no texture" << endl;
			}

			for (unsigned int i = 0; i < p_mesh->textures.size(); i++)
			{
				string number;
				string name = p_mesh->textures[i]->getDesc();

				if (name == "diffuse_map")
					number = std::to_string(diffuse_num++);
				//else if (name == "texture_specular")
				//	number = std::to_string(specularNr++); // transfer unsigned int to stream
				//else if (name == "texture_normal")
				//	number = std::to_string(normalNr++); // transfer unsigned int to stream
				//else if (name == "texture_height")
				//	number = std::to_string(heightNr++); // transfer unsigned int to stream

				uint32_t tex_id = p_mesh->textures[i]->getId();
				auto p = map_textureSampler.find(tex_id);
				if (p == map_textureSampler.end()) {
					optix::TextureSampler sampler = context->createTextureSamplerFromGLImage(tex_id, RT_TARGET_GL_TEXTURE_2D);
					sampler->setWrapMode(0, RT_WRAP_REPEAT);
					sampler->setWrapMode(1, RT_WRAP_REPEAT);
					map_textureSampler[tex_id] = sampler;
					material[name + number]->setTextureSampler(sampler);

				}
				else {
					material[name + number]->setTextureSampler(p->second);
				}

			}


			glm::vec3 Kd = p_mesh->mat.Kd;
			glm::vec3 Ks = p_mesh->mat.Ks;
			glm::vec3 Ka = p_mesh->mat.Ka;
			float Shininess = p_mesh->mat.Shininess;
			material["Kd"]->setFloat(optix::make_float3(Kd.r, Kd.g, Kd.b));
			material["Ks"]->setFloat(optix::make_float3(Ks.r, Ks.g, Ks.b));
			material["Ka"]->setFloat(optix::make_float3(Ka.r, Ka.g, Ka.b));
			material["Shininess"]->setFloat(Shininess);


			geom_instance = context->createGeometryInstance();
			geom_instance->setGeometry(g);
			geom_instance->addMaterial(material);
		}

		optix::GeometryInstance getMeshGeometry() {
			return geom_instance;
		}

		std::shared_ptr<Mesh> p_mesh;

		optix::Context               context;       // required

	private:
		MeshBuffers m_buffers;
		optix::Material              material;      // optional single matl override

		optix::Program               intersection;  // optional 
		optix::Program               bounds;        // optional
		optix::Program               attribute;        // optional

		optix::Program               closest_hit;   // optional multi matl override
		optix::Program               any_hit;       // optional
		optix::Program				 any_hit_shadow;


		// Output
		optix::GeometryInstance      geom_instance;
		//optix::float3                bbox_min;
		//optix::float3                bbox_max;

		unsigned int                 num_triangles;
	};
}
