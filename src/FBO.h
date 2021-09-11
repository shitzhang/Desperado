#pragma once



#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

#include "global.h"

enum class AttachmentType
{
	AttachmentType_Depth,
	AttachmentType_CubeDepth,
	AttachmentType_GBuffer,
	AttachmentType_NormalWorld,
	AttachmentType_Visibility,
	AttachmentType_PosWorld,
	AttachmentType_DiffuseColor,
};

class FBO
{
public:
	AttachmentType type;
	unsigned int resolution_width = 0;
	unsigned int resolution_height = 0;
	std::vector<unsigned int> textures;
	std::vector<unsigned int> attachments;  //使用的颜色附件单元
	FBO(AttachmentType type,unsigned int resolution_width,unsigned int resolution_height);
	~FBO();
	void init();
	void error();
	unsigned int CreateAndBindColorTargetTexture(unsigned int attachment);

	unsigned int getFboIdx();
private:
	unsigned int fbo_idx;
};


