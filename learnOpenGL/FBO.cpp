#include "FBO.h"

FBO::FBO() :GBufferNum(5)
{
	//width = SCR_WIDTH;
	//height = SCR_HEIGHT;
	init();
}

FBO::FBO(int GbuffNum) :GBufferNum(GbuffNum)
{
	//width = SCR_WIDTH;
	//height = SCR_HEIGHT;
	init();
	//return fbo;
}

FBO::FBO(int GbuffNum,int resolution) :GBufferNum(GbuffNum),resolution(resolution)
{
	//width = SCR_WIDTH;
	//height = SCR_HEIGHT;
	init();
	//return fbo;
}

FBO::~FBO()
{

}

void FBO::init() {
	//创建帧缓冲区对象
	glGenFramebuffers(1, &fbo_idx);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_idx);

	for (int i = 0; i < GBufferNum; i++) {
		unsigned int texture = CreateAndBindColorTargetTexture(i);
		attachments.push_back(i);
		textures.push_back(texture);
	}

	unsigned int depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	if (resolution) {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, resolution, resolution);
	}
	else {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, SCR_WIDTH, SCR_HEIGHT);
	}
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glBindRenderbuffer(GL_RENDERBUFFER, NULL);
}

void FBO::error() {

}

unsigned int FBO::CreateAndBindColorTargetTexture(unsigned int attachment) {
	//创建纹理对象并设置其尺寸和参数
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	if (resolution) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, texture, 0);
	return texture;
}