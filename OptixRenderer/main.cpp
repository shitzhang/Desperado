#include "stdafx.h"
#include<iostream>


	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void processInput(GLFWwindow* window);
	unsigned int loadTexture(char const* path);
	unsigned int loadCubemap(vector<std::string> faces);

	// camera

	shared_ptr<Desperado::Camera> camera = make_shared<Desperado::Camera>(glm::vec3(.0f, 10.0f, .0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);


	float lastX = SCR_WIDTH / 2.0f;
	float lastY = SCR_HEIGHT / 2.0f;
	bool firstMouse = true;


	// timing
	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrame = 0.0f;


	//test optix
	optix::Context        context = 0;
	uint32_t       width = SCR_WIDTH;
	uint32_t       height = SCR_HEIGHT;
	bool           use_pbo = true;

	int            frame_number = 1;
	int            sqrt_num_samples = 1;
	int            rr_begin_depth = 1;
	optix::Program        pgram_intersection = 0;
	optix::Program        pgram_bounding_box = 0;

	std::vector<optix::GeometryInstance> light_gis;

	// Camera state
	optix::float3         camera_up;
	optix::float3         camera_lookat;
	optix::float3         camera_eye;
	optix::Matrix4x4      camera_rotate;
	bool           camera_changed = true;
	
	//SVGF camera value
	glm::mat4 view_matrix = camera->GetViewMatrix();
	glm::mat4 projection_matrix = camera->GetPerspectiveMatrix();
	glm::mat4 pre_view_matrix;
	glm::mat4 pre_projection_matrix;


	optix::Buffer getOutputBuffer();
	void destroyContext();
	void registerExitHandler();
	void createContext();
	void setupLights();
	void loadGeometry();
	void setupCamera();
	void updateCamera();

	optix::Buffer getOutputBuffer()
	{
		return context["output_buffer"]->getBuffer();
	}

	void destroyContext()
	{
		if (context)
		{
			context->destroy();
			context = 0;
		}
	}



	void createContext()
	{
		context = optix::Context::create();
		context->setRayTypeCount(2);
		context->setEntryPointCount(1);
		context->setStackSize(1800);
		context->setMaxTraceDepth(2);

		// Set Output Debugging via rtPrintf
		context->setPrintEnabled(1);
		context->setPrintBufferSize(4096);

		//Buffer buffer = sutil::createOutputBuffer(context, RT_FORMAT_FLOAT4, width, height, use_pbo);
		optix::Buffer output_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);
		optix::Buffer output_direct_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);
		optix::Buffer output_indirect_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);

		optix::Buffer color_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);
		optix::Buffer direct_color_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);
		optix::Buffer indirect_color_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);	

		context["output_buffer"]->set(output_buffer);
		context["output_direct_buffer"]->set(output_direct_buffer);
		context["output_indirect_buffer"]->set(output_indirect_buffer);

		context["color_buffer"]->set(color_buffer);
		context["direct_color_buffer"]->set(direct_color_buffer);
		context["indirect_color_buffer"]->set(indirect_color_buffer);

		// Setup programs
		const char* ptx = optixUtil::getPtxString(SAMPLE_NAME, "optixSVGF.cu");
		context->setRayGenerationProgram(0, context->createProgramFromPTXString(ptx, "pathtrace_camera"));
		context->setExceptionProgram(0, context->createProgramFromPTXString(ptx, "exception"));
		context->setMissProgram(0, context->createProgramFromPTXString(ptx, "miss"));

		context["scene_epsilon"]->setFloat(1.e-3f);		
		context["rr_begin_depth"]->setUint(rr_begin_depth);
		context["sqrt_num_samples"]->setUint(sqrt_num_samples);
		context["bad_color"]->setFloat(1000000.0f, 0.0f, 1000000.0f); // Super magenta to make sure it doesn't get averaged out in the progressive rendering.		
		context["bg_color"]->setFloat(optix::make_float3(0.0f));
	}

	optix::GeometryInstance createParallelogram(
		const optix::float3& anchor,
		const optix::float3& offset1,
		const optix::float3& offset2)
	{
		optix::Geometry parallelogram = context->createGeometry();
		parallelogram->setPrimitiveCount(1u);
		parallelogram->setIntersectionProgram(pgram_intersection);
		parallelogram->setBoundingBoxProgram(pgram_bounding_box);

		optix::float3 normal = normalize(cross(offset1, offset2));
		float d = dot(normal, anchor);
		optix::float4 plane = make_float4(normal, d);

		optix::float3 v1 = offset1 / dot(offset1, offset1);
		optix::float3 v2 = offset2 / dot(offset2, offset2);

		parallelogram["plane"]->setFloat(plane);
		parallelogram["anchor"]->setFloat(anchor);
		parallelogram["v1"]->setFloat(v1);
		parallelogram["v2"]->setFloat(v2);

		optix::GeometryInstance gi = context->createGeometryInstance();
		gi->setGeometry(parallelogram);
		return gi;
	}

	void setupLights() {
		// Light buffer
		Desperado::OptixParallelogramLight light;
		light.corner = optix::make_float3(600.0f, 1000.0f, -100.0f);
		light.v1 = optix::make_float3(-1200.0f, 0.0f, 0.0f);
		light.v2 = optix::make_float3(0.0f, 0.0f, 200.0f);
		light.normal = normalize(cross(light.v1, light.v2));
		light.emission = optix::make_float3(15.0f, 15.0f, 15.0f);

		optix::Buffer light_buffer = context->createBuffer(RT_BUFFER_INPUT);
		light_buffer->setFormat(RT_FORMAT_USER);
		light_buffer->setElementSize(sizeof(Desperado::OptixParallelogramLight));
		light_buffer->setSize(1u);
		memcpy(light_buffer->map(), &light, sizeof(light));
		light_buffer->unmap();
		context["lights"]->setBuffer(light_buffer);

		optix::Material diffuse_light = context->createMaterial();
		std::string ptx = optixUtil::getPtxString(SAMPLE_NAME, "optixSVGF.cu");
		optix::Program closest_hit_emitter = context->createProgramFromPTXString(ptx, "closest_hit_emitter");
		diffuse_light->setClosestHitProgram(0, closest_hit_emitter);


		ptx = optixUtil::getPtxString(0, "parallelogram.cu");
		pgram_bounding_box = context->createProgramFromPTXString(ptx, "bounds");
		pgram_intersection = context->createProgramFromPTXString(ptx, "intersect");

		const optix::float3 light_em = optix::make_float3(15.0f, 15.0f, 15.0f);

		// Light
		light_gis.push_back(createParallelogram(light.corner, light.v1, light.v2));
		light_gis.back()->addMaterial(diffuse_light);
		light_gis.back()["emission_color"]->setFloat(light_em);
	}

	void setupCamera()
	{
		//camera_eye = make_float3(278.0f, 273.0f, -900.0f);
		//camera_lookat = make_float3(278.0f, 273.0f, 0.0f);
		//camera_up = make_float3(0.0f, 1.0f, 0.0f);

		//camera_rotate = Matrix4x4::identity();
	}


	void updateCamera()
	{
		const float fov = camera->Zoom;
		const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

		optix::float3 camera_u, camera_v, camera_w;

		float focal_length = (height / 2) / tanf(0.5f * fov * M_PIf / 180.0f);

		camera_u = optix::make_float3(camera->Right.x, camera->Right.y, camera->Right.z) * (float)width / 2;
		camera_v = optix::make_float3(camera->Up.x, camera->Up.y, camera->Up.z) * (float)height / 2;
		camera_w = optix::make_float3(camera->Front.x, camera->Front.y, camera->Front.z) * focal_length;

		camera_eye = optix::make_float3(camera->Position.x, camera->Position.y, camera->Position.z);

		if (camera_changed) // reset accumulation
			frame_number = 1;
		camera_changed = false;

		context["frame_number"]->setUint(frame_number++);
		context["eye"]->setFloat(camera_eye);
		context["U"]->setFloat(camera_u);
		context["V"]->setFloat(camera_v);
		context["W"]->setFloat(camera_w);
		//context["focal_length"]->setFloat(focal_length);
	}

	int main()
	{
		// glfw: initialize and configure
		// ------------------------------
		glfwInit();
		//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);            //限制了opengl版本为3.3，导致optix创建buffer出现问题，所以注释掉
		//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		// glfw window creation
		// --------------------
		GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OptixRenderer", NULL, NULL);

		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		// tell GLFW to capture our mouse
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glfwSwapInterval(0);
		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		auto screenShader = std::make_shared<Desperado::Shader>("../shader/framebuffers_screen.vs", "../shader/framebuffers_screen.fs");

		Desperado::TRStransform cornellTrans(glm::vec3(0.0, 0.0, 0.0));
		Desperado::TRStransform sponzaTrans(glm::vec3(0.0, 0.0, 0.0));
		Desperado::TRStransform MarryTrans(glm::vec3(0.0, 0.0, 0.0));
		Desperado::TRStransform nanoTrans(glm::vec3(0.0, 0.0, 0.0));

		//Model cornell_box("model/cornell_box/CornellBox-Empty-CO.obj", cornellTrans);
		auto sponza = make_shared<Desperado::Model>("../model/sponza/sponza.obj", sponzaTrans);
		//Model Mary("model/Marry/Marry.obj", MarryTrans);
		//Model nanosuit("model/nanosuit/nanosuit.obj", nanoTrans);

		auto pScene = std::make_shared<Desperado::Scene>(camera);
		pScene->AddModel(sponza);

		auto SVGFGbufferShader = std::make_shared<Desperado::Shader>("../shader/SVGF/SVGFGbuffer.vs", "../shader/SVGF/SVGFGbuffer.fs");

		Desperado::Fbo::Desc GbufferDesc;		
		GbufferDesc.setColorTarget(0, GL_RGBA32F, GL_RGBA);			// gPositionMeshId
		GbufferDesc.setColorTarget(1, GL_RG32F, GL_RG);				// gLinearZ
		GbufferDesc.setColorTarget(2, GL_RGB32F, GL_RGB);			// gNormal
		GbufferDesc.setColorTarget(3, GL_RG32F, GL_RG);			// gPositionNormalFwidth
		GbufferDesc.setColorTarget(4, GL_RGB32F, GL_RGB);			// gAlbedo
		GbufferDesc.setColorTarget(5, GL_RG32F, GL_RG);				// gMotion
		//GbufferDesc.setColorTarget(6, GL_R32F, GL_R);				// gMeshId
		GbufferDesc.setColorTarget(7, GL_RGB32F, GL_RGB);			// gEmission
		GbufferDesc.setDepthStencilTarget(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT);
		auto GbufferFbo = Desperado::Fbo::create2D(width, height, GbufferDesc);

		auto screenDisplayPass = Desperado::FullScreenPass::create();

		auto colorTex = Desperado::Texture::create2D(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
		auto directColorTex = Desperado::Texture::create2D(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
		auto indirectColorTex = Desperado::Texture::create2D(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
		//test ray tracing
		auto outputBufTex = Desperado::Texture::create2D(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
		auto outputDirectBufTex = Desperado::Texture::create2D(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
		auto outputIndirectBufTex = Desperado::Texture::create2D(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);

		auto outputTex = Desperado::Texture::create2D(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);

		auto svgfPass = Desperado::SVGFPass::create();
		try {
			//optix
			createContext();
			setupLights();
			// create geometry instances
			std::vector<optix::GeometryInstance> gis;

			for (auto p_mesh : sponza->p_meshes) {
				Desperado::OptiXMesh op_mesh(p_mesh, context);
				gis.push_back(op_mesh.getMeshGeometry());
			}

			// Create geometry group
			optix::GeometryGroup shadow_group = context->createGeometryGroup(gis.begin(), gis.end());
			shadow_group->setAcceleration(context->createAcceleration("Trbvh"));
			context["top_shadower"]->set(shadow_group);

			optix::GeometryGroup geometry_group = context->createGeometryGroup(gis.begin(), gis.end());
			for (auto light_gi : light_gis) {
				geometry_group->addChild(light_gi);
			}
			geometry_group->setAcceleration(context->createAcceleration("Trbvh"));
			context["top_object"]->set(geometry_group);

			context->validate();


			// render loop
			while (!glfwWindowShouldClose(window))
			{
				float currentFrame = glfwGetTime();
				deltaTime = currentFrame - lastFrame;
				lastFrame = currentFrame;

				processInput(window);
   
				updateCamera();
				context->launch(0, width, height);
				//glBindFramebuffer(GL_FRAMEBUFFER, GbufferFbo->getId());
				GbufferFbo->bind();

				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				glEnable(GL_DEPTH_TEST);
				SVGFGbufferShader->use();
				pScene->DrawModels(SVGFGbufferShader);

				auto colorBuffer = context["color_buffer"]->getBuffer();
				auto directColorBuffer = context["direct_color_buffer"]->getBuffer();
			    auto indirectColorBuffer = context["indirect_color_buffer"]->getBuffer();

				auto outputBuffer = context["output_buffer"]->getBuffer();
				auto outputDirectBuffer = context["output_direct_buffer"]->getBuffer();
				auto outputIndirectBuffer = context["output_indirect_buffer"]->getBuffer();

				optixUtil::displayBufferGL(colorBuffer, colorTex);
				optixUtil::displayBufferGL(directColorBuffer, directColorTex);
				optixUtil::displayBufferGL(indirectColorBuffer, indirectColorTex);

				optixUtil::displayBufferGL(outputBuffer, outputBufTex);
				optixUtil::displayBufferGL(outputDirectBuffer, outputDirectBufTex);
				optixUtil::displayBufferGL(outputIndirectBuffer, outputIndirectBufTex);

				auto posMeshIdTex = GbufferFbo->getColorTexture(0);
				auto linearZTex = GbufferFbo->getColorTexture(1);
				auto normalTex = GbufferFbo->getColorTexture(2);
				auto posNormalFwidthTex = GbufferFbo->getColorTexture(3);
				auto albedoTex = GbufferFbo->getColorTexture(4);
				auto motionTex = GbufferFbo->getColorTexture(5);
				//auto meshIDTex = GbufferFbo->getColorTexture(6);
				auto emissionTex = GbufferFbo->getColorTexture(7);

				auto renderData = Desperado::RenderData("SVGF", nullptr);
				renderData["gPositionMeshId"] = posMeshIdTex;
				renderData["gLinearZ"] = linearZTex;
				renderData["gNormal"] = normalTex;
				renderData["gPositionNormalFwidth"] = posNormalFwidthTex;
				renderData["gAlbedo"] = albedoTex;
				renderData["gMotion"] = motionTex;
				//renderData["gMeshId"] = meshIDTex;
				renderData["gEmission"] = emissionTex;

				renderData["color"] = directColorTex;
				renderData["directColor"] = directColorTex;
				renderData["indirectColor"] = indirectColorTex;
				renderData["output"] = outputTex;

				svgfPass->execute(renderData);

				//gamma correction
				//glEnable(GL_FRAMEBUFFER_SRGB);

				//glActiveTexture(GL_TEXTURE0);
				screenShader->use();
				//screenShader->setInt("screenTexture", 0);
				screenShader->setTexture2D("screenTexture", outputTex);
				//albedoTex->bind();

				{
					static unsigned frame_count = 0;
					displayFps(frame_count++, window);
				}

				screenDisplayPass->execute();

				// 交换缓冲并查询IO事件
				glfwSwapBuffers(window);
				glfwPollEvents();
			}

			glfwTerminate();
		}
		OPTIXUTIL_CATCH(context->get())
		return 0;
	}


	void processInput(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			//destroyContext();
			glfwSetWindowShouldClose(window, true);
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera->ProcessKeyboard(Desperado::FORWARD, deltaTime);
			camera_changed = true;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera->ProcessKeyboard(Desperado::BACKWARD, deltaTime);
			camera_changed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera->ProcessKeyboard(Desperado::LEFT, deltaTime);
			camera_changed = true;
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera->ProcessKeyboard(Desperado::RIGHT, deltaTime);
			camera_changed = true;
		}

	}


	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		// make sure the viewport matches the new window dimensions; note that width and 
		// height will be significantly larger than specified on retina displays.
		//SCR_WIDTH = width;
		//SCR_HEIGHT = height;
		glViewport(0, 0, width, height);
	}



	void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		camera->ProcessMouseMovement(xoffset, yoffset);
		camera_changed = true;
	}


	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		camera->ProcessMouseScroll(yoffset);
		camera_changed = true;
	}




