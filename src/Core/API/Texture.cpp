#include "stdafx.h"
#include "Texture.h"

namespace Desperado
{
    Texture::SharedPtr Texture::create1D(uint32_t width, GLenum internalFormat, GLenum format, uint32_t arraySize, uint32_t mipLevels, const void* pData)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, 1, 1, arraySize, mipLevels, 1, internalFormat,format, GL_TEXTURE_1D));
        pTexture->uploadInitData(pData);
        return pTexture;
    }

    Texture::SharedPtr Texture::create2D(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, uint32_t arraySize, uint32_t mipLevels, const void* pData)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, arraySize, mipLevels, 1, internalFormat,format, GL_TEXTURE_2D));
        pTexture->uploadInitData(pData);
        return pTexture;
    }

    Texture::SharedPtr Texture::create3D(uint32_t width, uint32_t height, uint32_t depth, GLenum internalFormat, GLenum format, uint32_t mipLevels, const void* pData)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, depth, 1, mipLevels, 1, internalFormat,format, GL_TEXTURE_3D));
        pTexture->uploadInitData(pData);
        return pTexture;
    }

    Texture::SharedPtr Texture::createCube(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, uint32_t arraySize, uint32_t mipLevels, const void* pData)
    {
        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, arraySize, mipLevels, 1, internalFormat,format, GL_TEXTURE_CUBE_MAP));
        pTexture->uploadInitData(pData);
        return pTexture;
    }

    Texture::SharedPtr Texture::createFromFile(const std::string& path, const string& directory, bool loadAsSrgb)
    {
        string filename = path;
        filename = directory + '/' + filename;

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

        GLenum internalFormat, format;
        if (nrComponents == 1) {
            internalFormat = GL_RGBA8;
            format = GL_RED;
        }
        else if (nrComponents == 3) {
            internalFormat = GL_RGBA8;
            format = GL_RGB;
        }
        else if (nrComponents == 4) {
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
        }

        Texture::SharedPtr pTexture = SharedPtr(new Texture(width, height, 1, 1, 0, 1, internalFormat, format, GL_TEXTURE_2D));

        if (data)
        {          
            pTexture->uploadInitData(data);           
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
        }
        stbi_image_free(data);
        return pTexture;
    }

    Texture::SharedPtr Texture::createConstant(const unsigned char* color, bool loadAsSrgb){       
        int width = 2;
        int height = 2;

        return create2D(width, height, GL_RGBA8, GL_RGBA, 1, 0, color);
    }

    Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, uint32_t mipLevels, uint32_t sampleCount, GLenum internalFormat ,GLenum format, GLenum type)
        : mWidth(width), mHeight(height), mDepth(depth), mMipLevels(mipLevels), mSampleCount(sampleCount), mArraySize(arraySize), mInternalFormat(internalFormat), mFormat(format),mType(type)
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

    void Texture::uploadInitData(const void* pData)
    {
        assert(pData);
        bind();
        glTexImage2D(mType, mMipLevels, mInternalFormat, mWidth, mHeight, 0, mFormat, GL_UNSIGNED_BYTE, pData);

        if (mMipLevels>0) {
            glGenerateMipmap(mType);
        }

        glTexParameteri(mType, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(mType, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
}
