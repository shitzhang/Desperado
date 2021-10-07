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

            if (mipLevel > pTexture->getMipCount())
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

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rtIndex, GL_TEXTURE_2D, pTexture->getId(), mipLevel);

        //glDrawBuffer(GL_COLOR_ATTACHMENT0 + rtIndex);
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
        auto fbState = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fbState == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            std::cout << "ERROR::FRAMEBUFFER:: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT!" << std::endl;
        }
        else if (fbState == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            std::cout << "ERROR::FRAMEBUFFER:: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT!" << std::endl;
        }
        else if (fbState == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
            std::cout << "ERROR::FRAMEBUFFER:: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER!" << std::endl;
        }
        else if (fbState == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) {
            std::cout << "ERROR::FRAMEBUFFER:: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER!" << std::endl;
        }
        else if (fbState == GL_FRAMEBUFFER_UNSUPPORTED) {
            std::cout << "ERROR::FRAMEBUFFER:: GL_FRAMEBUFFER_UNSUPPORTED!" << std::endl;
        }
        //for (auto const &attachment : mColorAttachments) {
        //    const Texture* pTexture = attachment.pTexture.get();
        //    if (pTexture != nullptr) {
        //        mWidth = std::min(mWidth, pTexture->getWidth(attachment.mipLevel));
        //        mHeight = std::min(mHeight, pTexture->getHeight(attachment.mipLevel));
        //        mDepth = std::min(mDepth, pTexture->getDepth(attachment.mipLevel));
        //    }
        //}
        //const Texture* pTexture = mDepthStencil.pTexture.get();
        //if (pTexture != nullptr) {
        //    mWidth = std::min(mWidth, pTexture->getWidth(mDepthStencil.mipLevel));
        //    mHeight = std::min(mHeight, pTexture->getHeight(mDepthStencil.mipLevel));
        //    mDepth = std::min(mDepth, pTexture->getDepth(mDepthStencil.mipLevel));
        //}
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

        std::vector<GLenum> attachments;
        // Create the color targets
        for (uint32_t i = 0; i < Fbo::getMaxColorTargetCount(); i++)
        {
            if (fboDesc.getColorTargetFormat(i) != 0)
            {              
                Texture::SharedPtr pTex = Texture::create2D(width, height, fboDesc.getColorTargetInternalFormat(i), fboDesc.getColorTargetFormat(i), GL_FLOAT, 1, mipLevels, nullptr);
                pFbo->attachColorTarget(pTex, i, 0);
            }
            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        }

        if (fboDesc.getDepthStencilFormat() != 0)
        {
            Texture::SharedPtr pDepthStencil = Texture::create2D(width, height, fboDesc.getDepthStencilInternalFormat(), fboDesc.getDepthStencilFormat(), GL_FLOAT, 1, mipLevels, nullptr);
            pFbo->attachDepthStencilTarget(pDepthStencil, 0);
        }

       
        glDrawBuffers(attachments.size(), &attachments[0]);

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
            auto pTex = Texture::createCube(width, height, fboDesc.getColorTargetFormat(i), fboDesc.getColorTargetFormat(i), GL_FLOAT, 1, mipLevels, nullptr);
            pFbo->attachColorTarget(pTex, i, 0);
        }

        if (fboDesc.getDepthStencilFormat() != 0)
        {
            auto pDepth = Texture::createCube(width, height, fboDesc.getDepthStencilFormat(), fboDesc.getDepthStencilFormat(), GL_FLOAT, 1, mipLevels, nullptr);
            pFbo->attachDepthStencilTarget(pDepth, 0);
        }

        pFbo->finalize();
        Fbo::unBind();

        return pFbo;
    }

    Fbo::Fbo()
    {
        mColorAttachments.resize(getMaxColorTargetCount());
        mWidth = SCR_WIDTH;
        mHeight = SCR_HEIGHT;
        mDepth = 1;
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

    void Fbo::clear() const
    {
        bind();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Fbo::blit(Texture::SharedPtr srcTex, Texture::SharedPtr dstTex)
    {
        auto blitFbo = getDefault();
        blitFbo->bind();
        //mColorAttachments.clear();
        blitFbo->mColorAttachments[0].pTexture = srcTex;
        blitFbo->mColorAttachments[0].mipLevel = 0;

        blitFbo->mColorAttachments[1].pTexture = dstTex;
        blitFbo->mColorAttachments[1].mipLevel = 0;

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex->getId(), 0);
        std::cout << glGetError() << std::endl;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, dstTex->getId(), 0);
        std::cout << glGetError() << std::endl;
        
        blitFbo->finalize();

        glReadBuffer(GL_COLOR_ATTACHMENT0); 

        //GLenum bufferlist[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffer(GL_COLOR_ATTACHMENT1);

        glBlitFramebuffer(0, 0, srcTex->getWidth(), srcTex->getHeight(), 0, 0, dstTex->getWidth(), dstTex->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST); // This will now copy from GL_COLOR_ATTACHMENT0 to GL_COLOR_ATTACHMENT1

        Fbo::unBind();
    }
    void Fbo::blit(Fbo::SharedPtr inFbo, int inAttachment, Texture::SharedPtr dstTex)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, inFbo->getId());
        int oldReadAttach;
        glGetIntegerv(GL_READ_BUFFER, &oldReadAttach);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + inAttachment);

        auto blitFbo = getDefault();

        //blitFbo->bind();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blitFbo->getId());

        //mColorAttachments.clear();
        blitFbo->mColorAttachments[0].pTexture = dstTex;

        blitFbo->mColorAttachments[0].mipLevel = 0;


        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTex->getId(), 0);


        blitFbo->finalize();

        //GLenum bufferlist[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(0, 0, inFbo->getColorTexture(inAttachment)->getWidth(), inFbo->getColorTexture(inAttachment)->getHeight(), 
            0, 0, dstTex->getWidth(), dstTex->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // cleanup
        //glReadBuffer(static_cast<GLenum>(oldReadAttach));
        Fbo::unBind();
    }
    void Fbo::blit(Fbo::SharedPtr inFbo, int inAttachment, Fbo::SharedPtr outFbo, int outAttachment)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, inFbo->getId());
        int oldReadAttach;
        glGetIntegerv(GL_READ_BUFFER, &oldReadAttach);

        glReadBuffer(GL_COLOR_ATTACHMENT0 + inAttachment);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, outFbo->getId());
        int oldDrawAttach;
        glGetIntegerv(GL_DRAW_BUFFER, &oldDrawAttach);

        glDrawBuffer(GL_COLOR_ATTACHMENT0 + outAttachment);

        glBlitFramebuffer(0, 0, inFbo->getColorTexture(inAttachment)->getWidth(), inFbo->getColorTexture(inAttachment)->getHeight(), 
            0, 0, outFbo->getColorTexture(outAttachment)->getWidth(), outFbo->getColorTexture(outAttachment)->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST); 

        // cleanup
        //glReadBuffer(static_cast<GLenum>(oldReadAttach));
        //glDrawBuffer(static_cast<GLenum>(oldDrawAttach));
        Fbo::unBind();
    }
}
