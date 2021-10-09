#include "stdafx.h"
#include "optixContext.h"

namespace Desperado {
	OptixContext::OptixContext(const std::string& sampleName, const std::string& cuName, Scene::SharedPtr pScene, Camera::SharedPtr pCamera, uint32_t width, uint32_t height)
		:mSampleName(sampleName), mCuName(cuName), mpScene(pScene), mpCamera(pCamera), m_width(width), m_height(height)
	{
		createContext();
		initProgram();
		setupLights();
		loadGeometry();
		setupCamera();
	}

	OptixContext::~OptixContext()
	{
		destroyContext();
	}

	optix::Buffer OptixContext::getBuffer(std::string bufName)
	{
		return m_context[bufName]->getBuffer();
	}

	void OptixContext::destroyContext()
	{
		if (m_context)
		{
			m_context->destroy();
			m_context = 0;
		}
	}

	void OptixContext::registerExitHandler()
	{
	}

	void OptixContext::createContext()
	{
		m_context = optix::Context::create();
		m_context->setRayTypeCount(2);
		m_context->setEntryPointCount(1);
		m_context->setStackSize(1800);
		m_context->setMaxTraceDepth(2);

		// Set Output Debugging via rtPrintf
		m_context->setPrintEnabled(1);
		m_context->setPrintBufferSize(4096);

		//Buffer buffer = sutil::createOutputBuffer(context, RT_FORMAT_FLOAT4, width, height, use_pbo);
		optix::Buffer output_buffer = m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);
		optix::Buffer output_direct_buffer = m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);
		optix::Buffer output_indirect_buffer = m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);

		optix::Buffer color_buffer = m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);
		optix::Buffer direct_color_buffer = m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);
		optix::Buffer indirect_color_buffer = m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, m_width, m_height);

		m_context["output_buffer"]->set(output_buffer);
		m_context["output_direct_buffer"]->set(output_direct_buffer);
		m_context["output_indirect_buffer"]->set(output_indirect_buffer);

		m_context["color_buffer"]->set(color_buffer);
		m_context["direct_color_buffer"]->set(direct_color_buffer);
		m_context["indirect_color_buffer"]->set(indirect_color_buffer);

		// Setup programs
		const char* ptx = optixUtil::getPtxString(mSampleName.c_str(), mCuName.c_str());
		m_context->setRayGenerationProgram(0, m_context->createProgramFromPTXString(ptx, "pathtrace_camera"));
		m_context->setExceptionProgram(0, m_context->createProgramFromPTXString(ptx, "exception"));
		m_context->setMissProgram(0, m_context->createProgramFromPTXString(ptx, "miss"));

		m_context["scene_epsilon"]->setFloat(1.e-3f);
		m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);
		m_context["sqrt_num_samples"]->setUint(m_sqrt_num_samples);
		m_context["bad_color"]->setFloat(1000000.0f, 0.0f, 1000000.0f); // Super magenta to make sure it doesn't get averaged out in the progressive rendering.		
		m_context["bg_color"]->setFloat(optix::make_float3(0.0f));
	}

	void OptixContext::setupLights()
	{
		//TODO: load from scene

		Desperado::OptixParallelogramLight light;
		light.corner = optix::make_float3(600.0f, 1000.0f, -100.0f);
		light.v1 = optix::make_float3(-1200.0f, 0.0f, 0.0f);
		light.v2 = optix::make_float3(0.0f, 0.0f, 200.0f);
		light.normal = normalize(cross(light.v1, light.v2));
		light.emission = optix::make_float3(15.0f, 15.0f, 15.0f);

		optix::Buffer light_buffer = m_context->createBuffer(RT_BUFFER_INPUT);
		light_buffer->setFormat(RT_FORMAT_USER);
		light_buffer->setElementSize(sizeof(Desperado::OptixParallelogramLight));
		light_buffer->setSize(1u);
		memcpy(light_buffer->map(), &light, sizeof(light));
		light_buffer->unmap();
		m_context["lights"]->setBuffer(light_buffer);


		const optix::float3 light_em = optix::make_float3(15.0f, 15.0f, 15.0f);

		// Light
		m_light_gis.push_back(createParallelogram(light.corner, light.v1, light.v2));
		m_light_gis.back()->addMaterial(m_diffuse_light);
		m_light_gis.back()["emission_color"]->setFloat(light_em);
	}

	void OptixContext::loadGeometry()
	{
		auto models = mpScene->getModels();
		for (auto& p_model : models) {
			auto meshes = p_model->p_meshes;
			for (auto& p_mesh : meshes) {
				unsigned int num_triangles = p_mesh->indices.size() / 3;

				auto vbo = p_mesh->getVBO();
				auto ebo = p_mesh->getEBO();

				MeshBuffers buffers;

				buffers.vertices = m_context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getVBO());
				buffers.vertices->setFormat(RT_FORMAT_USER);

				buffers.vertices->setElementSize(sizeof(Vertex));
				buffers.vertices->setSize(p_mesh->vertices.size());

				buffers.indices = m_context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getEBO());
				buffers.indices->setFormat(RT_FORMAT_UNSIGNED_INT3);
				buffers.indices->setSize(num_triangles);

				optix::Geometry g = m_context->createGeometry();
				g["vertex_buffer"]->setBuffer(buffers.vertices);
				g["index_buffer"]->setBuffer(buffers.indices);
				g["mesh_ID"]->setUint(p_mesh->getMeshID());
				g->setPrimitiveCount(num_triangles);
				g->setIntersectionProgram(m_triangle_intersection);
				g->setBoundingBoxProgram(m_triangle_bounds);

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

					auto tex = p_mesh->textures[i];
					auto p = m_texture_sampler.find(tex);
					if (p == m_texture_sampler.end()) {
						optix::TextureSampler sampler = m_context->createTextureSamplerFromGLImage(tex->getId(), RT_TARGET_GL_TEXTURE_2D);
						sampler->setWrapMode(0, RT_WRAP_REPEAT);
						sampler->setWrapMode(1, RT_WRAP_REPEAT);
						m_texture_sampler[tex] = sampler;
						m_diffuse[name + number]->setTextureSampler(sampler);

					}
					else {
						m_diffuse[name + number]->setTextureSampler(p->second);
					}

				}


				glm::vec3 Kd = p_mesh->mat.Kd;
				glm::vec3 Ks = p_mesh->mat.Ks;
				glm::vec3 Ka = p_mesh->mat.Ka;
				float Shininess = p_mesh->mat.Shininess;
				m_diffuse["Kd"]->setFloat(optix::make_float3(Kd.r, Kd.g, Kd.b));
				m_diffuse["Ks"]->setFloat(optix::make_float3(Ks.r, Ks.g, Ks.b));
				m_diffuse["Ka"]->setFloat(optix::make_float3(Ka.r, Ka.g, Ka.b));
				m_diffuse["Shininess"]->setFloat(Shininess);


				optix::GeometryInstance geom_instance = m_context->createGeometryInstance();
				geom_instance->setGeometry(g);
				geom_instance->addMaterial(m_diffuse);

				m_gis.push_back(geom_instance);
			}
		}
		optix::GeometryGroup shadow_group = m_context->createGeometryGroup(m_gis.begin(), m_gis.end());
		shadow_group->setAcceleration(m_context->createAcceleration("Trbvh"));
		m_context["top_shadower"]->set(shadow_group);

		optix::GeometryGroup geometry_group = m_context->createGeometryGroup(m_gis.begin(), m_gis.end());
		for (auto light_gi : m_light_gis) {
			geometry_group->addChild(light_gi);
		}
		geometry_group->setAcceleration(m_context->createAcceleration("Trbvh"));
		m_context["top_object"]->set(geometry_group);
	}

	void OptixContext::setupCamera()
	{
		//camera_eye = make_float3(278.0f, 273.0f, -900.0f);
		//camera_lookat = make_float3(278.0f, 273.0f, 0.0f);
		//camera_up = make_float3(0.0f, 1.0f, 0.0f);
		//camera_rotate = Matrix4x4::identity();
	}

	void OptixContext::updateCamera()
	{
		const float fov = mpCamera->Zoom;
		const float aspect_ratio = static_cast<float>(m_width) / static_cast<float>(m_height);

		optix::float3 camera_u, camera_v, camera_w;

		float focal_length = (m_height / 2) / tanf(0.5f * fov * M_PIf / 180.0f);

		camera_u = optix::make_float3(mpCamera->Right.x, mpCamera->Right.y, mpCamera->Right.z) * (float)m_width / 2;
		camera_v = optix::make_float3(mpCamera->Up.x, mpCamera->Up.y, mpCamera->Up.z) * (float)m_height / 2;
		camera_w = optix::make_float3(mpCamera->Front.x, mpCamera->Front.y, mpCamera->Front.z) * focal_length;

		m_camera_eye = optix::make_float3(mpCamera->Position.x, mpCamera->Position.y, mpCamera->Position.z);

		if (m_camera_changed) // reset accumulation
			m_frame_number = 1;
		m_camera_changed = false;

		m_context["frame_number"]->setUint(m_frame_number++);
		m_context["eye"]->setFloat(m_camera_eye);
		m_context["U"]->setFloat(camera_u);
		m_context["V"]->setFloat(camera_v);
		m_context["W"]->setFloat(camera_w);
		//m_context["focal_length"]->setFloat(focal_length);
	}

	void OptixContext::validate()
	{
		m_context->validate();
	}

	void OptixContext::launch()
	{
		m_context->launch(0, m_width, m_height);
	}

	optix::GeometryInstance OptixContext::createParallelogram(const optix::float3& anchor, const optix::float3& offset1, const optix::float3& offset2)
	{
		optix::Geometry parallelogram = m_context->createGeometry();
		parallelogram->setPrimitiveCount(1u);
		parallelogram->setIntersectionProgram(m_pgram_intersection);
		parallelogram->setBoundingBoxProgram(m_pgram_bounding_box);

		optix::float3 normal = normalize(cross(offset1, offset2));
		float d = dot(normal, anchor);
		optix::float4 plane = make_float4(normal, d);

		optix::float3 v1 = offset1 / dot(offset1, offset1);
		optix::float3 v2 = offset2 / dot(offset2, offset2);

		parallelogram["plane"]->setFloat(plane);
		parallelogram["anchor"]->setFloat(anchor);
		parallelogram["v1"]->setFloat(v1);
		parallelogram["v2"]->setFloat(v2);

		optix::GeometryInstance gi = m_context->createGeometryInstance();
		gi->setGeometry(parallelogram);
		return gi;
	}

	void OptixContext::initProgram()
	{
		const char* ptx = optixUtil::getPtxString(0, "triangle_mesh.cu");
		m_triangle_intersection = m_context->createProgramFromPTXString(ptx, "mesh_intersect");
		m_triangle_bounds = m_context->createProgramFromPTXString(ptx, "mesh_bounds");

		ptx = optixUtil::getPtxString(0, "parallelogram.cu");
		m_pgram_bounding_box = m_context->createProgramFromPTXString(ptx, "bounds");
		m_pgram_intersection = m_context->createProgramFromPTXString(ptx, "intersect");

		m_diffuse = m_context->createMaterial();
		ptx = optixUtil::getPtxString(mSampleName.c_str(), mCuName.c_str());
		m_closest_hit = m_context->createProgramFromPTXString(ptx, "closest_hit");
		m_any_hit = m_context->createProgramFromPTXString(ptx, "any_hit");
		m_any_hit_shadow = m_context->createProgramFromPTXString(ptx, "any_hit_shadow");
		m_diffuse->setClosestHitProgram(0, m_closest_hit);
		m_diffuse->setAnyHitProgram(0, m_any_hit);
		m_diffuse->setAnyHitProgram(1, m_any_hit_shadow);

		m_diffuse_light = m_context->createMaterial();
		ptx = optixUtil::getPtxString(mSampleName.c_str(), mCuName.c_str());
		m_closest_hit_emitter = m_context->createProgramFromPTXString(ptx, "closest_hit_emitter");
		m_diffuse_light->setClosestHitProgram(0, m_closest_hit_emitter);
	}

	void OptixContext::initSceneGeometry()
	{
	}

	void OptixContext::initSceneLights()
	{
	}

	void OptixContext::initSceneCamera()
	{
	}
}
