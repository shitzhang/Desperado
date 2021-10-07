
#pragma once
#include "Core/API/Texture.h"
#include "Core/API/Formats.h"
#include <memory>
#include <glad/glad.h>

namespace Desperado
{
    class Fbo : public std::enable_shared_from_this<Fbo>
    {
    public:
        using SharedPtr = std::shared_ptr<Fbo>;
        using SharedConstPtr = std::shared_ptr<const Fbo>;

        class Desc
        {
        public:
            Desc();
            Desc& setColorTarget(uint32_t rtIndex, GLenum internalFormat, GLenum format) { mColorTargets[rtIndex] = TargetDesc(internalFormat, format); return *this; }
            Desc& setDepthStencilTarget(GLenum internalFormat, GLenum format) { mDepthStencilTarget = TargetDesc(internalFormat, format); return *this; }

            GLenum getColorTargetInternalFormat(uint32_t rtIndex) const { return mColorTargets[rtIndex].internalFormat; }
            GLenum getColorTargetFormat(uint32_t rtIndex) const { return mColorTargets[rtIndex].format; }
            GLenum getDepthStencilInternalFormat() const { return mDepthStencilTarget.internalFormat; }
            GLenum getDepthStencilFormat() const { return mDepthStencilTarget.format; }

            bool operator==(const Desc& other) const;

        private:
            struct TargetDesc
            {
                TargetDesc() = default;
                TargetDesc(GLenum inf, GLenum f) : internalFormat(inf), format(f) {}

                GLenum internalFormat = 0;
                GLenum format = 0;
                
                bool operator==(const TargetDesc& other) const { return ((format == other.format) && (internalFormat == other.internalFormat)); }
                bool operator!=(const TargetDesc& other) const { return !(*this == other); }
            };

            std::vector<TargetDesc> mColorTargets;
            TargetDesc mDepthStencilTarget;
        };

        ~Fbo();

        static SharedPtr getDefault();
        static SharedPtr create();
        static SharedPtr create(const std::vector<Texture::SharedPtr>& colors, const Texture::SharedPtr& pDepth = nullptr);
        static SharedPtr create2D(uint32_t width, uint32_t height, const Desc& fboDesc, uint32_t mipLevels = 0);
        static SharedPtr createCubemap(uint32_t width, uint32_t height, const Desc& fboDesc, uint32_t mipLevels = 0);

        void attachDepthStencilTarget(const Texture::SharedPtr& pDepthStencil, uint32_t mipLevel = 0);

        void attachColorTarget(const Texture::SharedPtr& pColorTexture, uint32_t rtIndex, uint32_t mipLevel = 0);

        static uint32_t getMaxColorTargetCount();

        Texture::SharedPtr getColorTexture(uint32_t index) const;

        const Texture::SharedPtr& getDepthStencilTexture() const;


        uint32_t getWidth() const { finalize(); return mWidth; }
        uint32_t getHeight() const { finalize(); return mHeight; }

        const Desc& getDesc() const { finalize();  return *mpDesc; }

        struct Attachment
        {
            Texture::SharedPtr pTexture = nullptr;
            uint32_t mipLevel = 0;
        };

        void bind() const;
        static void unBind(); // Unbinds whatever current framebuffer is bound
        void clear() const;

        unsigned int getId() const { return mId; }
        static void blit(Texture::SharedPtr srcTex, Texture::SharedPtr dstTex);
        static void blit(Fbo::SharedPtr inFbo, int inAttachment, Texture::SharedPtr dstTex);
        static void blit(Fbo::SharedPtr inFbo, int inAttachment, Fbo::SharedPtr outFbo, int outAttachment);

        
    private:
        void applyColorAttachment(uint32_t rtIndex);
        void applyDepthAttachment();
        void initApiHandle() const;

        //Validates that the framebuffer attachments are OK.
        void finalize() const;

        Fbo();
        std::vector<Attachment> mColorAttachments;
        Attachment mDepthStencil;

        mutable const Desc* mpDesc = nullptr;
        mutable uint32_t mWidth  = (uint32_t)-1;
        mutable uint32_t mHeight = (uint32_t)-1;
        mutable uint32_t mDepth = (uint32_t)-1;

        //void* mpPrivateData = nullptr;
        uint32_t mId = 0;
    };
}
