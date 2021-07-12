#pragma once



#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

#include "global.h"

enum class AttachmentType
{
	AttachmentType_Depth,
	AttachmentType_NormalWorld,
	AttachmentType_Visibility,
	AttachmentType_PosWorld,
	AttachmentType_DiffuseColor,
};

class FBO
{
public:
	unsigned int fbo_idx;
	int GBufferNum;
	int resolution = 0;
	std::vector<unsigned int> textures;
	std::vector<unsigned int> attachments;  //使用的颜色附件单元
	FBO();
	FBO(int GbuffNum);
	FBO(int GbuffNum, int resolution);
	~FBO();
	void init();
	void error();
	unsigned int CreateAndBindColorTargetTexture(unsigned int attachment);
private:

};


