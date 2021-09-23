
#pragma once

#include "Core/Framework.h"
#define _USE_MATH_DEFINES
#include <math.h>
//
////GL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
////image
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//
#include "global.h"
#include "shader.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "model.h"
//
////optix
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h> 
#include <optixu/optixu_math_stream_namespace.h>
//
#include "optixUtil.h"
#include "optixMesh.h"
#include "optixLight.h"

// Core
//#include "Core/Sample.h"
//#include "Core/Window.h"

// Core/API
//#include "Core/API/BlendState.h"
//#include "Core/API/Buffer.h"
//#include "Core/API/ComputeContext.h"
//#include "Core/API/ComputeStateObject.h"
//#include "Core/API/CopyContext.h"
//#include "Core/API/DepthStencilState.h"
//#include "Core/API/DescriptorPool.h"
//#include "Core/API/DescriptorSet.h"
//#include "Core/API/Device.h"
#include "Core/API/FBO.h"
//#include "Core/API/FencedPool.h"
//#include "Core/API/Formats.h"
//#include "Core/API/GpuFence.h"
//#include "Core/API/GpuTimer.h"
//#include "Core/API/GraphicsStateObject.h"
//#include "Core/API/LowLevelContextData.h"
//#include "Core/API/QueryHeap.h"
//#include "Core/API/RasterizerState.h"
//#include "Core/API/RenderContext.h"
//#include "Core/API/Resource.h"
//#include "Core/API/GpuMemoryHeap.h"
//#include "Core/API/ResourceViews.h"
//#include "Core/API/RootSignature.h"
//#include "Core/API/Sampler.h"
#include "Core/API/Texture.h"
//#include "Core/API/VAO.h"
//#include "Core/API/VertexLayout.h"

// Core/BufferTypes
//#include "Core/BufferTypes/ParameterBlock.h"
//#include "Core/BufferTypes/VariablesBufferUI.h"

// Core/Platform
//#include "Core/Platform/OS.h"
//#include "Core/Platform/ProgressBar.h"

// Core/Program
//#include "Core/Program/ComputeProgram.h"
//#include "Core/Program/GraphicsProgram.h"
//#include "Core/Program/CUDAProgram.h"
//#include "Core/Program/Program.h"
//#include "Core/Program/ProgramReflection.h"
//#include "Core/Program/ProgramVars.h"
//#include "Core/Program/ProgramVersion.h"
//#include "Core/Program/ShaderLibrary.h"

// Core/State
//#include "Core/State/ComputeState.h"
//#include "Core/State/GraphicsState.h"

// RenderGraph
//#include "RenderGraph/RenderGraph.h"
//#include "RenderGraph/RenderGraphImportExport.h"
//#include "RenderGraph/RenderGraphIR.h"
//#include "RenderGraph/RenderGraphUI.h"
//#include "RenderGraph/RenderPass.h"
//#include "RenderGraph/RenderPassLibrary.h"
//#include "RenderGraph/RenderPassReflection.h"
//#include "RenderGraph/RenderPassStandardFlags.h"
//#include "RenderGraph/ResourceCache.h"
//#include "RenderGraph/BasePasses/ComputePass.h"
//#include "RenderGraph/BasePasses/RasterPass.h"
//#include "RenderGraph/BasePasses/RasterScenePass.h"
//#include "RenderGraph/BasePasses/FullScreenPass.h"

// Scene
//#include "Scene/Scene.h"
//#include "Scene/Importer.h"
//#include "Scene/Camera/Camera.h"
//#include "Scene/Camera/CameraController.h"
//#include "Scene/Lights/Light.h"
//#include "Scene/Material/Material.h"
//#include "Scene/Animation/Animation.h"
//#include "Scene/Animation/AnimationController.h"
//#include "Scene/ParticleSystem/ParticleSystem.h"

// Utils
//#include "Utils/Math/AABB.h"
//#include "Utils/BinaryFileStream.h"
//#include "Utils/CryptoUtils.h"
//#include "Utils/Logger.h"
//#include "Utils/NumericRange.h"
//#include "Utils/StringUtils.h"
//#include "Utils/TermColor.h"
//#include "Utils/Threading.h"
//#include "Utils/Algorithm/DirectedGraph.h"
//#include "Utils/Algorithm/DirectedGraphTraversal.h"
//#include "Utils/Algorithm/ParallelReduction.h"
//#include "Utils/Image/Bitmap.h"
//#include "Utils/Image/ImageIO.h"
//#include "Utils/Math/CubicSpline.h"
//#include "Utils/Math/FalcorMath.h"
//#include "Utils/Scripting/Dictionary.h"
//#include "Utils/Perception/Experiment.h"
//#include "Utils/Perception/SingleThresholdMeasurement.h"
//#include "Utils/SampleGenerators/DxSamplePattern.h"
//#include "Utils/SampleGenerators/HaltonSamplePattern.h"
//#include "Utils/SampleGenerators/StratifiedSamplePattern.h"
//#include "Utils/SampleGenerators/CPUSampleGenerator.h"
//#include "Utils/Scripting/Scripting.h"
//#include "Utils/Scripting/Console.h"
//#include "Utils/Timing/CpuTimer.h"
//#include "Utils/Timing/Clock.h"
//#include "Utils/Timing/FrameRate.h"
//#include "Utils/Timing/Profiler.h"
//#include "Utils/Timing/ProfilerUI.h"
//#include "Utils/Timing/TimeReport.h"
//#include "Utils/UI/Font.h"
//#include "Utils/UI/Gui.h"
//#include "Utils/UI/DebugDrawer.h"
//#include "Utils/UI/PixelZoom.h"
//#include "Utils/UI/TextRenderer.h"
//#include "Utils/UI/UserInput.h"
//#include "Utils/Video/VideoEncoder.h"
//#include "Utils/Video/VideoEncoderUI.h"
//#include "Utils/Debug/DebugConsole.h"
//#include "Utils/Debug/PixelDebug.h"


#define DESPERADO_MAJOR_VERSION 1
#define DESPERADO_REVISION 0
#define DESPERADO_VERSION_STRING "1.0"
