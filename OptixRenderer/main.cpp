#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "global.h"
#include "shader.h"
#include "scene.h"
#include "camera.h"
//#include "light.h"
#include "model.h"

#include "SceneDepthPass.h"
#include "simpleShadowPass.h"

#include<iostream>

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h> 
#include <optixu/optixu_math_stream_namespace.h>

#include "optixUtil.h"
#include "optixMesh.h"
#include "optixLight.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(vector<std::string> faces);

// camera
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

shared_ptr<Camera> camera = make_shared<Camera>(glm::vec3(.0f, 10.0f, .0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);


float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


//test optix
//using namespace optix;
//const char* const SAMPLE_NAME = "optixPathTracer";

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

	context["scene_epsilon"]->setFloat(1.e-3f);
	context["rr_begin_depth"]->setUint(rr_begin_depth);
	context["sqrt_num_samples"]->setUint(sqrt_num_samples);
	//Buffer buffer = sutil::createOutputBuffer(context, RT_FORMAT_FLOAT4, width, height, use_pbo);
	optix::Buffer buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);
	context["output_buffer"]->set(buffer);

	// Setup programs
	const char* ptx = optixUtil::getPtxString(SAMPLE_NAME, "optixPathTracer.cu");
	context->setRayGenerationProgram(0, context->createProgramFromPTXString(ptx, "pathtrace_camera"));
	context->setExceptionProgram(0, context->createProgramFromPTXString(ptx, "exception"));
	context->setMissProgram(0, context->createProgramFromPTXString(ptx, "miss"));

	//context["sqrt_num_samples"]->setUint(sqrt_num_samples);
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
	OptixParallelogramLight light;
	light.corner = optix::make_float3(0.0f, 800.0f, 0.0f);
	light.v1 = optix::make_float3(-800.0f, 0.0f, 0.0f);
	light.v2 = optix::make_float3(0.0f, 0.0f, 200.0f);
	light.normal = normalize(cross(light.v1, light.v2));
	light.emission = optix::make_float3(15.0f, 15.0f, 15.0f);

	optix::Buffer light_buffer = context->createBuffer(RT_BUFFER_INPUT);
	light_buffer->setFormat(RT_FORMAT_USER);
	light_buffer->setElementSize(sizeof(OptixParallelogramLight));
	light_buffer->setSize(1u);
	memcpy(light_buffer->map(), &light, sizeof(light));
	light_buffer->unmap();
	context["lights"]->setBuffer(light_buffer);

	optix::Material diffuse_light = context->createMaterial();
	std::string ptx = optixUtil::getPtxString(SAMPLE_NAME, "optixPathTracer.cu");
	optix::Program closest_hit_emitter = context->createProgramFromPTXString(ptx, "closest_hit_emitter");
	diffuse_light->setClosestHitProgram(0, closest_hit_emitter);


	ptx = optixUtil::getPtxString(SAMPLE_NAME, "parallelogram.cu");
	pgram_bounding_box = context->createProgramFromPTXString(ptx, "bounds");
	pgram_intersection = context->createProgramFromPTXString(ptx, "intersect");

	const optix::float3 light_em = optix::make_float3(15.0f, 15.0f, 5.0f);

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

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
	};
	// screen quad VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	Shader screenShader("shader/5.1.framebuffers_screen.vs", "shader/5.1.framebuffers_screen.fs");
	unsigned int screenTexID = 0;

	TRStransform cornellTrans(glm::vec3(0.0, 0.0, 0.0));
	TRStransform sponzaTrans(glm::vec3(0.0, 0.0, 0.0));
	TRStransform MarryTrans(glm::vec3(0.0, 0.0, 0.0));
	TRStransform nanoTrans(glm::vec3(0.0, 0.0, 0.0));
	TRStransform bedTrans(glm::vec3(0.0, 0.0, 0.0));

	//Model cornell_box("model/cornell_box/CornellBox-Empty-CO.obj", cornellTrans);
	Model sponza("model/sponza/sponza.obj", sponzaTrans);
	//Model Mary("model/Marry/Marry.obj", MarryTrans);
	//Model nanosuit("model/nanosuit/nanosuit.obj", nanoTrans);
	//Model bedroom("model/bedroom/iscv2.obj", bedTrans);
	//Model breakfast_room("model/breakfast_room/breakfast_room.obj", bedTrans);
	//Model sibenik("model/sibenik/sibenik.obj", sponzaTrans);
	

	try {
		//optix
		createContext();
		setupLights();
		// create geometry instances
		std::vector<optix::GeometryInstance> gis;


		for (auto p_mesh : sponza.p_meshes) {

			OptiXMesh op_mesh(p_mesh, context);

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


		// 渲染循环
		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			// 输入
			processInput(window);

			// render
			// ------       
			updateCamera();
			context->launch(0, width, height);

			optixUtil::displayBufferGL(getOutputBuffer(), screenTexID);

			//gamma correction
			glEnable(GL_FRAMEBUFFER_SRGB);

			glActiveTexture(GL_TEXTURE0);
			screenShader.use();
			screenShader.setInt("screenTexture", 0);
			glBindTexture(GL_TEXTURE_2D, screenTexID);

			{
				//static unsigned frame_count = 0;
				//optixUtil::displayFps(frame_count++);
			}

			// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
			// clear all relevant buffers
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
			glClear(GL_COLOR_BUFFER_BIT);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

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
		camera->ProcessKeyboard(FORWARD, deltaTime);
		camera_changed = true;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera->ProcessKeyboard(BACKWARD, deltaTime);
		camera_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera->ProcessKeyboard(LEFT, deltaTime);
		camera_changed = true;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera->ProcessKeyboard(RIGHT, deltaTime);
		camera_changed = true;
	}

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
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



