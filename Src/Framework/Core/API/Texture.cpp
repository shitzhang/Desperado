#include "stdafx.h"
#include "Texture.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Desperado
{
	Texture::SharedPtr Texture::create1D(uint32_t width, GLenum internalFormat, GLenum format, GLenum dataFormat,
	                                     uint32_t arraySize, uint32_t mipLevels, const void* pData)
	{
		Texture::SharedPtr pTexture = SharedPtr(new Texture(width, 1, 1, arraySize, mipLevels, 1, internalFormat,
		                                                    format, GL_TEXTURE_1D));
		pTexture->uploadInitData(pData, dataFormat);
		return pTexture;
	}

	Texture::SharedPtr Texture::create2D(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format,
	                                     GLenum dataFormat, uint32_t arraySize, uint32_t mipLevels, const void* pData)
	{
		Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, arraySize, mipLevels, 1, internalFormat,
		                                                    format, GL_TEXTURE_2D));
		pTexture->uploadInitData(pData, dataFormat);
		return pTexture;
	}

	Texture::SharedPtr Texture::create3D(uint32_t width, uint32_t height, uint32_t depth, GLenum internalFormat,
	                                     GLenum format, GLenum dataFormat, uint32_t mipLevels, const void* pData)
	{
		Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, depth, 1, mipLevels, 1, internalFormat,
		                                                    format, GL_TEXTURE_3D));
		pTexture->uploadInitData(pData, dataFormat);
		return pTexture;
	}

	Texture::SharedPtr Texture::createCube(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format,
	                                       GLenum dataFormat, uint32_t arraySize, uint32_t mipLevels, const void* pData)
	{
		Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, arraySize, mipLevels, 1, internalFormat,
		                                                    format, GL_TEXTURE_CUBE_MAP));
		pTexture->uploadInitData(pData, dataFormat);
		return pTexture;
	}

	Texture::SharedPtr Texture::createFromFile(const std::string& path, const std::string& directory, bool loadAsSrgb,
	                                           GLenum dataFormat)
	{
		string filename = path;
		filename = directory + '/' + filename;

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

		GLenum internalFormat, format;
		if (nrComponents == 1)
		{
			internalFormat = GL_RGBA8;
			format = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = GL_RGBA8;
			format = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = GL_RGBA8;
			format = GL_RGBA;
		}

		Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, 1, 0, 1, internalFormat, format,
		                                                    GL_TEXTURE_2D));

		if (data)
		{
			pTexture->uploadInitData(data, dataFormat);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
		}
		stbi_image_free(data);
		return pTexture;
	}

	Texture::SharedPtr Texture::createConstant(const unsigned char* color, bool loadAsSrgb, GLenum dataFormat)
	{
		int width = 2;
		int height = 2;

		return create2D(width, height, GL_RGBA8, GL_RGBA, dataFormat, 1, 0, color);
	}

	Texture::SharedPtr Texture::createFromPBO(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format,
	                                          GLenum dataFormat, const uint32_t pboId, uint32_t arraySize,
	                                          uint32_t mipLevels)
	{
		if (pboId == 0)
		{
			std::cout << "Texture: pboId can't be zero" << std::endl;
		}
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);

		if (dataFormat == GL_FLOAT)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		else if (dataFormat == GL_UNSIGNED_INT)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		else if (dataFormat == GL_UNSIGNED_BYTE)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		Texture::SharedPtr pTexture = create2D(width, height, internalFormat, format, dataFormat, arraySize, mipLevels,
		                                       0);

		if (pboId)
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		return pTexture;
	}

	Texture::SharedPtr Texture::biltFromPBO(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format,
	                                        GLenum dataFormat, const uint32_t pboId, uint32_t arraySize,
	                                        uint32_t mipLevels)
	{
		if (pboId == 0)
		{
			std::cout << "Texture: pboId can't be zero" << std::endl;
		}
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);

		if (dataFormat == GL_FLOAT)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		else if (dataFormat == GL_UNSIGNED_INT)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		else if (dataFormat == GL_UNSIGNED_BYTE)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		mWidth = width;
		mHeight = height;
		mInternalFormat = internalFormat;
		mFormat = format;
		mArraySize = arraySize;
		mMipLevels = mipLevels;

		uploadInitData(0, dataFormat);

		if (pboId)
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		return shared_from_this();
	}

	void Texture::biltFromCPU(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum dataFormat,
	                          uint32_t arraySize, uint32_t mipLevels, const void* pInitData)
	{
		mWidth = width;
		mHeight = height;
		mInternalFormat = internalFormat;
		mFormat = format;
		mArraySize = arraySize;
		mMipLevels = mipLevels;

		uploadInitData(pInitData, dataFormat);

		//return shared_from_this();
	}

	Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, uint32_t mipLevels,
	                 uint32_t sampleCount, GLenum internalFormat, GLenum format, GLenum type)
		: mWidth(width), mHeight(height), mDepth(depth), mMipLevels(mipLevels), mSampleCount(sampleCount),
		  mArraySize(arraySize), mInternalFormat(internalFormat), mFormat(format), mType(type)
	{
		assert(width > 0 && height > 0 && depth > 0);
		//assert(arraySize > 0 && sampleCount > 0);
		assert(internalFormat != GL_NONE && format != GL_NONE);

		glGenTextures(1, &mId);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &mId);
	}

	void Texture::captureToFile(uint32_t mipLevel, uint32_t arraySlice, const std::string& filename)
	{
	}

	void Texture::uploadInitData(const void* pData, GLenum dataFormat)
	{
		//may be need to seperate filter for load obj and display optix buffer
		//assert(pData);
		//if (pData == nullptr) {
		//    return;
		//}
		bind();
		glTexImage2D(mType, mMipLevels, mInternalFormat, mWidth, mHeight, 0, mFormat, dataFormat, pData);

		if (mMipLevels > 0)
		{
			glGenerateMipmap(mType);
		}

		glTexParameteri(mType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(mType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		unBind();
	}


	uint64_t Texture::getTexelCount() const
	{
		uint64_t count = 0;
		for (uint32_t i = 0; i < getMipCount(); i++)
		{
			uint64_t texelsInMip = (uint64_t)getWidth(i) * getHeight(i) * getDepth(i);
			assert(texelsInMip > 0);
			count += texelsInMip;
		}
		count *= getArraySize();
		assert(count > 0);
		return count;
	}

	void Texture::writeToFile(std::string filename)
	{
		unsigned char* bytes = new unsigned char[4 * mWidth * mHeight];
		float* data = new float[4 * mWidth * mHeight];

		glBindTexture(GL_TEXTURE_2D, mId);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);

		for (uint32_t h1 = 0; h1 < mHeight / 2; ++h1)
		{
			uint32_t h2 = mHeight - 1 - h1;
			for (uint32_t i = 0; i < 4 * mWidth; ++i)
			{
				std::swap(data[h1 * mWidth * 4 + i], data[h2 * mWidth * 4 + i]);
			}
		}

		for (uint32_t h = 0; h < mHeight; ++h)
		{
			for (uint32_t i = 0; i < 4 * mWidth; ++i)
			{
				float temp = glm::clamp(data[h * mWidth * 4 + i], 0.f, 1.f);
				bytes[h * mWidth * 4 + i] = static_cast<unsigned char>(temp * 255.0f);
			}
		}

		stbi_write_png(filename.c_str(), mWidth, mHeight, 4, bytes, mWidth * 4);
		std::cout << "Saved " << filename << "." << std::endl;

		delete[] bytes;
	}
}
