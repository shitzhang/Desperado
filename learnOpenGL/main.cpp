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
#include "light.h"
#include "model.h"

#include "SceneDepthPass.h"
#include "simpleShadowPass.h"

#include<iostream>

#include<optixu/optixpp_namespace.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(vector<std::string> faces);

// camera
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

shared_ptr<Camera> camera = make_shared<Camera>(glm::vec3(30.0f, 30.0f, 30.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "learnOpenGL", NULL, NULL);
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


	camera->pFbo = make_shared<FBO>();

	glm::vec3 lightRadiance = glm::vec3(1.0, 1.0, 1.0);
	glm::vec3 lightPos = glm::vec3(0.0, 80.0, 80.0);

	glm::vec3 lightDir = glm::vec3(0.0, -80.0, -80.0);

	glm::vec3 lightUp = glm::vec3(0.0, 1.0, 0.0);

	TRStransform lightTrans = TRStransform(lightPos, glm::vec3(1.0, 1.0, 1.0));
	Mesh lightCube = cube(lightTrans);
	Shader lightShader("shader/lightShader/lightCube.vs", "shader/lightShader/lightCube.fs");


	auto light = make_shared<DirectionalLight>(lightRadiance, lightPos, lightDir, lightUp, lightCube, lightShader);

	auto scene = make_shared<Scene>();
	scene->AddLight(light);

	TRStransform maryTrans1(glm::vec3(0.0, 0.0, 0.0), glm::vec3(20.0, 20.0, 20.0));

	auto mary1 = make_shared<Model>("model/Marry/Marry.obj", maryTrans1);

	scene->AddModel(mary1);

	TRStransform maryTrans2(glm::vec3(40.0, 0.0, -40.0), glm::vec3(10.0, 10.0, 10.0));

	auto mary2 = make_shared<Model>("model/Marry/Marry.obj", maryTrans2);

	scene->AddModel(mary2);

	TRStransform floorTrans(glm::vec3(0.0, 0.0, -30.0), glm::vec3(4.0, 4.0, 4.0));

	auto floor = make_shared<Model>("model/floor/floor.obj", floorTrans);

	scene->AddModel(floor);

	scene->pCamera = camera;


	//passes
	SceneDepthPass depthPass(scene, camera);
	SimpleShadowPass simpleShadowPass(scene, camera);

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
		depthPass.render();
		simpleShadowPass.render();

		// 交换缓冲并查询IO事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(yoffset);
}



