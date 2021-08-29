#include "FBO.h"


FBO::FBO(AttachmentType type,unsigned int resolution_width,unsigned int resolution_height) :
	type(type), resolution_width(resolution_width), resolution_height(resolution_height)
{
	init();
}

FBO::~FBO()
{

}

unsigned int FBO::getFboIdx() {
	return fbo_idx;
}

void FBO::init() {
	//创建帧缓冲区对象
	glGenFramebuffers(1, &fbo_idx);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_idx);

	switch (type)
	{
	case AttachmentType::AttachmentType_Depth:
		break;
	case AttachmentType::AttachmentType_CubeDepth:
		glGenFramebuffers(1, &fbo_idx);
		// create depth cubemap texture
		unsigned int depthCubemap;
		glGenTextures(1, &depthCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, resolution_width, resolution_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_idx);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	case AttachmentType::AttachmentType_GBuffer:
		for (int i = 0; i < 5; i++) {
			unsigned int texture = CreateAndBindColorTargetTexture(i);
			attachments.push_back(i);
			textures.push_back(texture);
		}

		unsigned int depthBuffer;
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		if (resolution_width && resolution_height) {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, resolution_width, resolution_height);
		}
		else {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, SCR_WIDTH, SCR_HEIGHT);
		}

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D, NULL);
		glBindRenderbuffer(GL_RENDERBUFFER, NULL);
		break;
	case AttachmentType::AttachmentType_NormalWorld:
		break;
	case AttachmentType::AttachmentType_Visibility:
		break;
	case AttachmentType::AttachmentType_PosWorld:
		break;
	case AttachmentType::AttachmentType_DiffuseColor:
		break;
	default:
		break;
	}
}

void FBO::error() {

}

unsigned int FBO::CreateAndBindColorTargetTexture(unsigned int attachment) {
	//创建纹理对象并设置其尺寸和参数
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	if (resolution_width&&resolution_height) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution_width, resolution_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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