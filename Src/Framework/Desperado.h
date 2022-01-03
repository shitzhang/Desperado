#pragma once

// Core
#include "Core/Framework.h"
#include "Core/Sample.h"
#include "Core/Window.h"

// GL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Image
#include "stb_image.h"

#include "Global.h"
#include "Shader.h"
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"

// Optix
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h> 
#include <optixu/optixu_math_stream_namespace.h>

#include "Optix/OptixUtil.h"
#include "Optix/OptixMesh.h"
#include "Optix/OptixLight.h"
#include "Optix/OptixContext.h"

// Core/API
#include "Core/API/FBO.h"
#include "Core/API/Texture.h"
#include "Core/API/RenderContext.h"

// RenderPass
#include "Passes/RenderPass.h"
#include "Passes/FullScreenPass.h"
#include "Passes/SVGFPass.h"

// Scene
//#include "Scene/Scene.h"

// Utils
#include "Utils/Math/Vector.h"
#include "Utils/InternalDictionary.h"
#include "Utils/Threading.h"
#include "Utils/StringUtils.h"
#include "Utils/UI/UserInput.h"
#include "Utils/UI/Gui.h"
#include "Utils/Timing/CpuTimer.h"
#include "Utils/Timing/Clock.h"
#include "Utils/Timing/FrameRate.h"
#include "Utils/Video/VideoEncoder.h"
#include "Utils/Video/VideoEncoderUI.h"


#define DESPERADO_MAJOR_VERSION 1
#define DESPERADO_REVISION 0
#define DESPERADO_VERSION_STRING "1.0"
