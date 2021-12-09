/***************************************************************************
 # Copyright (c) 2015-21, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#include <map>

namespace Desperado
{
    class Texture: public std::enable_shared_from_this<Texture>
    {
    public:
        using SharedPtr = std::shared_ptr<Texture>;
        using SharedConstPtr = std::shared_ptr<const Texture>;
        using WeakPtr = std::weak_ptr<Texture>;
        using WeakConstPtr = std::weak_ptr<const Texture>;

        ~Texture();

        void bind() { glBindTexture(mType, mId); };
        void unBind() { glBindTexture(mType, 0); };

        uint32_t getId() const { return mId; }
        GLenum getType() const { return mType; }

        void setDesc(const std::string& desc) { mDesc = desc; }
        std::string getDesc() const { return mDesc; }

        uint32_t getWidth(uint32_t mipLevel = 0) const { return (mipLevel == 0) || (mipLevel < mMipLevels) ? std::max(1U, mWidth >> mipLevel) : 0; }
        uint32_t getHeight(uint32_t mipLevel = 0) const { return (mipLevel == 0) || (mipLevel < mMipLevels) ? std::max(1U, mHeight >> mipLevel) : 0; }
        uint32_t getDepth(uint32_t mipLevel = 0) const { return (mipLevel == 0) || (mipLevel < mMipLevels) ? std::max(1U, mDepth >> mipLevel) : 0; }

        uint32_t getMipCount() const { return mMipLevels; }
        uint32_t getSampleCount() const { return mSampleCount; }

        uint32_t getArraySize() const { return mArraySize; }
        uint32_t getSubresourceArraySlice(uint32_t subresource) const { return subresource / mMipLevels; }
        uint32_t getSubresourceMipLevel(uint32_t subresource) const { return subresource % mMipLevels; }
        uint32_t getSubresourceIndex(uint32_t arraySlice, uint32_t mipLevel) const { return mipLevel + arraySlice * mMipLevels; }

        GLenum getInternalFormat() const { return mInternalFormat; }
        GLenum getFormat() const { return mFormat; }

        static SharedPtr create1D(uint32_t width, GLenum internalFormat, GLenum format, GLenum dataFormat, uint32_t arraySize = 1, uint32_t mipLevels = 0, const void* pInitData = nullptr);
        static SharedPtr create2D(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum dataFormat, uint32_t arraySize = 1, uint32_t mipLevels = 0, const void* pInitData = nullptr);
        static SharedPtr create3D(uint32_t width, uint32_t height, uint32_t depth, GLenum internalFormat, GLenum format, GLenum dataFormat, uint32_t mipLevels = 0, const void* pInitData = nullptr);
        static SharedPtr createCube(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum dataFormat, uint32_t arraySize = 1, uint32_t mipLevels = 0, const void* pInitData = nullptr);
        static SharedPtr createFromFile(const std::string& path, const std::string& directory, bool loadAsSrgb, GLenum dataFormat = GL_UNSIGNED_BYTE);
        static SharedPtr createConstant(const unsigned char* color, bool loadAsSrgb, GLenum dataFormat = GL_UNSIGNED_BYTE);
        static SharedPtr createFromPBO(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum dataFormat, const uint32_t pboId, uint32_t arraySize = 1, uint32_t mipLevels = 0);

        SharedPtr biltFromPBO(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum dataFormat, const uint32_t pboId, uint32_t arraySize = 1, uint32_t mipLevels = 0);
        void biltFromCPU(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum dataFormat, uint32_t arraySize = 1, uint32_t mipLevels = 0, const void* pInitData = nullptr);

        void captureToFile(uint32_t mipLevel, uint32_t arraySlice, const std::string& filename);

        void setPath(const std::string& path) { mPath = path; }
        const std::string& getPath() const { return mPath; }

        uint64_t getTexelCount() const;

    	void writeToFile(std::string filename);

    protected:
        Texture(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, uint32_t mipLevels, uint32_t sampleCount, GLenum internalFormat,GLenum format, GLenum type);
        void uploadInitData(const void* pData, GLenum dataFormat);

        std::string mPath;
        GLenum mType;
        std::string mDesc;
        uint32_t mId;

        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        uint32_t mDepth = 0;
        uint32_t mMipLevels = 0;
        uint32_t mSampleCount = 0;
        uint32_t mArraySize = 0;
        GLenum mInternalFormat = 0;
        GLenum mFormat = 0;
    };
}
