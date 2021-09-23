#include "stdafx.h"
#include "FBO.h"

namespace Desperado
{
    namespace
    {
        bool checkAttachmentParams(const Texture* pTexture, uint32_t mipLevel)
        {
#ifndef _DEBUG
            return true;
#endif
            if (pTexture == nullptr)
            {
                return true;
            }

            if (mipLevel >= pTexture->getMipCount())
            {
                std::cout << ("Error when attaching texture to FBO. Requested mip-level is out-of-bound.") << std::endl;
                return false;
            }
            return true;
        }

        bool checkParams(const std::string& Func, uint32_t width, uint32_t height, uint32_t mipLevels)
        {
            std::string msg = "Fbo::" + Func + "() - ";
            std::string param;

            if      (width == 0)     param = "width";
            else if (height == 0)    param = "height";
            else
            {
                return true;
            }

            std::cout<<(msg + param + " can't be zero.")<<std::endl;
            return false;
        }
    };


    bool Fbo::Desc::operator==(const Fbo::Desc& other) const
    {
        if (mColorTargets.size() != other.mColorTargets.size()) return false;

        for (size_t i = 0; i < mColorTargets.size(); i++)
        {
            if (mColorTargets[i] != other.mColorTargets[i]) return false;
        }
        if (mDepthStencilTarget != other.mDepthStencilTarget) return false;

        return true;
    }

    Fbo::Desc::Desc()
    {
        mColorTargets.resize(Fbo::getMaxColorTargetCount());
    }

    Fbo::SharedPtr Fbo::create()
    {
        return SharedPtr(new Fbo());
    }

    Fbo::SharedPtr Fbo::create(const std::vector<Texture::SharedPtr>& colors, const Texture::SharedPtr& pDepth)
    {
        auto pFbo = create();

        pFbo->bind();
        for (uint32_t i = 0 ; i < colors.size() ; i++)
        {
            pFbo->attachColorTarget(colors[i], i);
        }
        if (pDepth)
        {
            pFbo->attachDepthStencilTarget(pDepth);
        }

        Fbo::unBind();
        pFbo->finalize();
        return pFbo;
    }

    Fbo::SharedPtr Fbo::getDefault()
    {
        static Fbo::SharedPtr pDefault;
        if (pDefault == nullptr)
        {
            pDefault = Fbo::SharedPtr(new Fbo());
        }
        return pDefault;
    }

    void Fbo::attachDepthStencilTarget(const Texture::SharedPtr& pDepthStencil, uint32_t mipLevel)
    {
        if (checkAttachmentParams(pDepthStencil.get(), mipLevel) == false)
        {
            throw std::exception("Can't attach depth-stencil texture to FBO. Invalid parameters.");
        }

        mpDesc = nullptr;
        mDepthStencil.pTexture = pDepthStencil;
        mDepthStencil.mipLevel = mipLevel;

        switch (pDepthStencil->getFormat()) {
        case GL_DEPTH_COMPONENT:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pDepthStencil->getId(), mipLevel);
            break;
        case  GL_STENCIL_INDEX:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, pDepthStencil->getId(), mipLevel);
            break;
        case GL_DEPTH_STENCIL:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, pDepthStencil->getId(), mipLevel);
            break;
        default:
            break;
        }
    }

    void Fbo::attachColorTarget(const Texture::SharedPtr& pTexture, uint32_t rtIndex, uint32_t mipLevel)
    {
        if (rtIndex >= mColorAttachments.size())
        {
            throw std::exception(("Error when attaching texture to FBO. Requested color index " + std::to_string(rtIndex) + ", but context only supports " + std::to_string(mColorAttachments.size()) + " targets").c_str());
        }
        if (checkAttachmentParams(pTexture.get(), mipLevel) == false)
        {
            throw std::exception("Can't attach texture to FBO. Invalid parameters.");
        }

        mpDesc = nullptr;
        mColorAttachments[rtIndex].pTexture = pTexture;
        mColorAttachments[rtIndex].mipLevel = mipLevel;

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture->getId(), mipLevel);
    }

    uint32_t Fbo::getMaxColorTargetCount()
    {
        GLint maxAttach = 0;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
        return maxAttach;
    }


    Texture::SharedPtr Fbo::getColorTexture(uint32_t index) const
    {
        if (index >= mColorAttachments.size())
        {
            throw std::exception(("Can't get texture from FBO. Index is out of range. Requested " + std::to_string(index) + " but only " + std::to_string(mColorAttachments.size()) + " color slots are available.").c_str());
        }
        return mColorAttachments[index].pTexture;
    }

    const Texture::SharedPtr& Fbo::getDepthStencilTexture() const
    {
        return mDepthStencil.pTexture;
    }

    void Fbo::finalize() const
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        for (auto const &attachment : mColorAttachments) {
            const Texture* pTexture = attachment.pTexture.get();
            mWidth = std::min(mWidth, pTexture->getWidth(attachment.mipLevel));
            mHeight = std::min(mHeight, pTexture->getHeight(attachment.mipLevel));
            mDepth = std::min(mDepth, pTexture->getDepth(attachment.mipLevel));
        }
        const Texture* pTexture = mDepthStencil.pTexture.get();
        mWidth = std::min(mWidth, pTexture->getWidth(mDepthStencil.mipLevel));
        mHeight = std::min(mHeight, pTexture->getHeight(mDepthStencil.mipLevel));
        mDepth = std::min(mDepth, pTexture->getDepth(mDepthStencil.mipLevel));
    }

    Fbo::SharedPtr Fbo::create2D(uint32_t width, uint32_t height, const Fbo::Desc& fboDesc, uint32_t mipLevels)
    {
        if (checkParams("Create2D", width, height, mipLevels) == false)
        {
            throw std::exception("Can't create 2D FBO. Invalid parameters.");
        }

        Fbo::SharedPtr pFbo = create();

        pFbo->mWidth = std::min(pFbo->mWidth, width >> mipLevels);
        pFbo->mHeight = std::min(pFbo->mHeight, height >> mipLevels);
        pFbo->mDepth = 1;

        pFbo->bind();
        // Create the color targets
        for (uint32_t i = 0; i < Fbo::getMaxColorTargetCount(); i++)
        {
            if (fboDesc.getColorTargetFormat(i) != 0)
            {              
                Texture::SharedPtr pTex = Texture::create2D(width, height, fboDesc.getColorTargetInternalFormat(i), fboDesc.getColorTargetFormat(i), 1, mipLevels, nullptr);
                pFbo->attachColorTarget(pTex, i, 0);
            }
        }

        if (fboDesc.getDepthStencilFormat() != 0)
        {
            Texture::SharedPtr pDepth = Texture::create2D(width, height, fboDesc.getDepthStencilInternalFormat(), fboDesc.getDepthStencilFormat(), 1, mipLevels, nullptr);
            pFbo->attachDepthStencilTarget(pDepth, 0);
        }

        pFbo->finalize();

        Fbo::unBind();

        return pFbo;
    }

    Fbo::SharedPtr Fbo::createCubemap(uint32_t width, uint32_t height, const Desc& fboDesc, uint32_t mipLevels)
    {

        if (checkParams("CreateCubemap", width, height, mipLevels) == false)
        {
            throw std::exception("Can't create cubemap FBO. Invalid parameters.");
        }

        Fbo::SharedPtr pFbo = create();

        pFbo->mWidth = std::min(pFbo->mWidth, width >> mipLevels);
        pFbo->mHeight = std::min(pFbo->mHeight, height >> mipLevels);
        pFbo->mDepth = 1;

        pFbo->bind();
        // Create the color targets
        for (uint32_t i = 0; i < getMaxColorTargetCount(); i++)
        {
            auto pTex = Texture::createCube(width, height, fboDesc.getColorTargetFormat(i), fboDesc.getColorTargetFormat(i), 1, mipLevels, nullptr);
            pFbo->attachColorTarget(pTex, i, 0);
        }

        if (fboDesc.getDepthStencilFormat() != 0)
        {
            auto pDepth = Texture::createCube(width, height, fboDesc.getDepthStencilFormat(), fboDesc.getDepthStencilFormat(), 1, mipLevels, nullptr);
            pFbo->attachDepthStencilTarget(pDepth, 0);
        }

        pFbo->finalize();
        Fbo::unBind();

        return pFbo;
    }

    Fbo::Fbo()
    {
        mColorAttachments.resize(getMaxColorTargetCount());
        glGenFramebuffers(1, &mId);
    }

    Fbo::~Fbo()
    {
        glDeleteFramebuffers(1, &mId);
    }

    void Fbo::applyColorAttachment(uint32_t rtIndex)
    {
    }

    void Fbo::applyDepthAttachment()
    {
    }
    void Fbo::initApiHandle() const
    {
    }

    void Fbo::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, mId);
        //glViewport(0, 0, mWidth, mHeight);
    }

    void Fbo::unBind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    uint32_t Fbo::getId() const { return mId; }
}
