#include "stdafx.h"
#include "SVGFPass.h"

/*
TODO:
- clean up shaders
- clean up UI: tooltips, etc.
- handle skybox pixels
- enum for fbo channel indices
*/
namespace Desperado {
    namespace
    {
        // Shader source files
        const char kPackLinearZAndNormalShader[] = "RenderPasses/SVGFPass/SVGFPackLinearZAndNormal.ps.slang";
        const char kReprojectShader[] = "RenderPasses/SVGFPass/SVGFReproject.ps.slang";
        const char kAtrousShader[] = "RenderPasses/SVGFPass/SVGFAtrous.ps.slang";
        const char kFilterMomentShader[] = "RenderPasses/SVGFPass/SVGFFilterMoments.ps.slang";
        const char kFinalModulateShader[] = "RenderPasses/SVGFPass/SVGFFinalModulate.ps.slang";

        // Names of valid entries in the parameter dictionary.
        const char kEnabled[] = "Enabled";
        const char kIterations[] = "Iterations";
        const char kFeedbackTap[] = "FeedbackTap";
        const char kVarianceEpsilon[] = "VarianceEpsilon";
        const char kPhiColor[] = "PhiColor";
        const char kPhiNormal[] = "PhiNormal";
        const char kAlpha[] = "Alpha";
        const char kMomentsAlpha[] = "MomentsAlpha";

        // Input buffer names
        const char kInputBufferAlbedo[] = "Albedo";
        const char kInputBufferColor[] = "Color";
        const char kInputBufferEmission[] = "Emission";
        const char kInputBufferWorldPosition[] = "WorldPosition";
        const char kInputBufferWorldNormal[] = "WorldNormal";
        const char kInputBufferPosNormalFwidth[] = "PositionNormalFwidth";
        const char kInputBufferLinearZ[] = "LinearZ";
        const char kInputBufferMotionVector[] = "MotionVec";

        // Internal buffer names
        const char kInternalBufferPreviousLinearZAndNormal[] = "Previous Linear Z and Packed Normal";
        const char kInternalBufferPreviousLighting[] = "Previous Lighting";
        const char kInternalBufferPreviousMoments[] = "Previous Moments";

        // Output buffer name
        const char kOutputBufferFilteredImage[] = "Filtered image";
    }

    SVGFPass::SharedPtr SVGFPass::create(const InternalDictionary& dict)
    {
        return SharedPtr(new SVGFPass(dict));
    }

    SVGFPass::SVGFPass(const InternalDictionary& dict)
    {
        for (const auto& [key, value] : dict)
        {
            if (key == kEnabled) mFilterEnabled = value;
            else if (key == kIterations) mFilterIterations = value;
            else if (key == kFeedbackTap) mFeedbackTap = value;
            else if (key == kVarianceEpsilon) mVarainceEpsilon = value;
            else if (key == kPhiColor) mPhiColor = value;
            else if (key == kPhiNormal) mPhiNormal = value;
            else if (key == kAlpha) mAlpha = value;
            else if (key == kMomentsAlpha) mMomentsAlpha = value;
            else std::cout<<("Unknown field '" + key + "' in SVGFPass dictionary")<<std::endl;
        }

        mpPackLinearZAndNormal = FullScreenPass::create();
        mpReprojection = FullScreenPass::create();
        mpAtrous = FullScreenPass::create();
        mpFilterMoments = FullScreenPass::create();
        mpFinalModulate = FullScreenPass::create();
        assert(mpPackLinearZAndNormal && mpReprojection && mpAtrous && mpFilterMoments && mpFinalModulate);
    }

    /*
    Reproject:
      - takes: motion, color, prevLighting, prevMoments, linearZ, prevLinearZ, historyLen
        returns: illumination, moments, historyLength
    Variance/filter moments:
      - takes: illumination, moments, history length, normal+depth
      - returns: filtered illumination+variance (to ping pong fbo)
    a-trous:
      - takes: albedo, filtered illumination+variance, normal+depth, history length
      - returns: final color
    */

    void SVGFPass::execute(const RenderData& renderData)
    {
        Texture::SharedPtr pAlbedoTexture = renderData[kInputBufferAlbedo]->asTexture();
        Texture::SharedPtr pColorTexture = renderData[kInputBufferColor]->asTexture();
        Texture::SharedPtr pEmissionTexture = renderData[kInputBufferEmission]->asTexture();
        Texture::SharedPtr pWorldPositionTexture = renderData[kInputBufferWorldPosition]->asTexture();
        Texture::SharedPtr pWorldNormalTexture = renderData[kInputBufferWorldNormal]->asTexture();
        Texture::SharedPtr pPosNormalFwidthTexture = renderData[kInputBufferPosNormalFwidth]->asTexture();
        Texture::SharedPtr pLinearZTexture = renderData[kInputBufferLinearZ]->asTexture();
        Texture::SharedPtr pMotionVectorTexture = renderData[kInputBufferMotionVector]->asTexture();

        Texture::SharedPtr pOutputTexture = renderData[kOutputBufferFilteredImage]->asTexture();

        assert(mpFilteredIlluminationFbo &&
            mpFilteredIlluminationFbo->getWidth() == pAlbedoTexture->getWidth() &&
            mpFilteredIlluminationFbo->getHeight() == pAlbedoTexture->getHeight());

        if (mBuffersNeedClear)
        {
            clearBuffers(pRenderContext, renderData);
            mBuffersNeedClear = false;
        }

        if (mFilterEnabled)
        {
            // Grab linear z and its derivative and also pack the normal into
            // the last two channels of the mpLinearZAndNormalFbo.
            computeLinearZAndNormal(pRenderContext, pLinearZTexture, pWorldNormalTexture);

            // Demodulate input color & albedo to get illumination and lerp in
            // reprojected filtered illumination from the previous frame.
            // Stores the result as well as initial moments and an updated
            // per-pixel history length in mpCurReprojFbo.
            Texture::SharedPtr pPrevLinearZAndNormalTexture =
                renderData[kInternalBufferPreviousLinearZAndNormal]->asTexture();
            computeReprojection(pRenderContext, pAlbedoTexture, pColorTexture, pEmissionTexture,
                pMotionVectorTexture, pPosNormalFwidthTexture,
                pPrevLinearZAndNormalTexture);

            // Do a first cross-bilateral filtering of the illumination and
            // estimate its variance, storing the result into a float4 in
            // mpPingPongFbo[0].  Takes mpCurReprojFbo as input.
            computeFilteredMoments(pRenderContext);

            // Filter illumination from mpCurReprojFbo[0], storing the result
            // in mpPingPongFbo[0].  Along the way (or at the end, depending on
            // the value of mFeedbackTap), save the filtered illumination for
            // next time into mpFilteredPastFbo.
            computeAtrousDecomposition(pRenderContext, pAlbedoTexture);

            // Compute albedo * filtered illumination and add emission back in.
            auto perImageCB = mpFinalModulate["PerImageCB"];
            perImageCB["gAlbedo"] = pAlbedoTexture;
            perImageCB["gEmission"] = pEmissionTexture;
            perImageCB["gIllumination"] = mpPingPongFbo[0]->getColorTexture(0);
            mpFinalModulate->execute(pRenderContext, mpFinalFbo);

            // Blit into the output texture.
            pRenderContext->blit(mpFinalFbo->getColorTexture(0)->getSRV(), pOutputTexture->getRTV());

            // Swap resources so we're ready for next frame.
            std::swap(mpCurReprojFbo, mpPrevReprojFbo);
            pRenderContext->blit(mpLinearZAndNormalFbo->getColorTexture(0)->getSRV(),
                pPrevLinearZAndNormalTexture->getRTV());
        }
        else
        {
            pRenderContext->blit(pColorTexture->getSRV(), pOutputTexture->getRTV());
        }
    }

    void SVGFPass::allocateFbos(uint2 dim)
    {
        {
            // Screen-size FBOs with 3 MRTs: one that is RGBA32F, one that is
            // RG32F for the luminance moments, and one that is R16F.
            Fbo::Desc desc;
            desc.setColorTarget(0, GL_RGBA32F, GL_RGBA); // illumination
            desc.setColorTarget(1, GL_RG32F, GL_RG);   // moments
            desc.setColorTarget(2, GL_R16F, GL_R);    // history length
            mpCurReprojFbo = Fbo::create2D(dim.x, dim.y, desc);
            mpPrevReprojFbo = Fbo::create2D(dim.x, dim.y, desc);
        }

        {
            // Screen-size RGBA32F buffer for linear Z, derivative, and packed normal
            Fbo::Desc desc;
            desc.setColorTarget(0, GL_RGBA32F, GL_RGBA);
            mpLinearZAndNormalFbo = Fbo::create2D(dim.x, dim.y, desc);
        }

        {
            // Screen-size FBOs with 1 RGBA32F buffer
            Fbo::Desc desc;
            desc.setColorTarget(0, GL_RGBA32F, GL_RGBA);
            mpPingPongFbo[0] = Fbo::create2D(dim.x, dim.y, desc);
            mpPingPongFbo[1] = Fbo::create2D(dim.x, dim.y, desc);
            mpFilteredPastFbo = Fbo::create2D(dim.x, dim.y, desc);
            mpFilteredIlluminationFbo = Fbo::create2D(dim.x, dim.y, desc);
            mpFinalFbo = Fbo::create2D(dim.x, dim.y, desc);
        }

        mBuffersNeedClear = true;
    }

    void SVGFPass::clearBuffers(RenderContext* pRenderContext, const RenderData& renderData)
    {
        pRenderContext->clearFbo(mpPingPongFbo[0].get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        pRenderContext->clearFbo(mpPingPongFbo[1].get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        pRenderContext->clearFbo(mpLinearZAndNormalFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        pRenderContext->clearFbo(mpFilteredPastFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        pRenderContext->clearFbo(mpCurReprojFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        pRenderContext->clearFbo(mpPrevReprojFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        pRenderContext->clearFbo(mpFilteredIlluminationFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);

        pRenderContext->clearTexture(renderData[kInternalBufferPreviousLinearZAndNormal]->asTexture().get());
        pRenderContext->clearTexture(renderData[kInternalBufferPreviousLighting]->asTexture().get());
        pRenderContext->clearTexture(renderData[kInternalBufferPreviousMoments]->asTexture().get());
    }

    // Extracts linear z and its derivative from the linear Z texture and packs
    // the normal from the world normal texture and packes them into the FBO.
    // (It's slightly wasteful to copy linear z here, but having this all
    // together in a single buffer is a small simplification, since we make a
    // copy of it to refer to in the next frame.)
    void SVGFPass::computeLinearZAndNormal(Texture::SharedPtr pLinearZTexture,
        Texture::SharedPtr pWorldNormalTexture)
    {
        auto perImageCB = mpPackLinearZAndNormal["PerImageCB"];
        perImageCB["gLinearZ"] = pLinearZTexture;
        perImageCB["gNormal"] = pWorldNormalTexture;

        mpPackLinearZAndNormal->execute(mpLinearZAndNormalFbo);
    }

    void SVGFPass::computeReprojection(Texture::SharedPtr pAlbedoTexture,
        Texture::SharedPtr pColorTexture, Texture::SharedPtr pEmissionTexture,
        Texture::SharedPtr pMotionVectorTexture,
        Texture::SharedPtr pPositionNormalFwidthTexture,
        Texture::SharedPtr pPrevLinearZTexture)
    {
        auto perImageCB = mpReprojection["PerImageCB"];

        // Setup textures for our reprojection shader pass
        perImageCB["gMotion"] = pMotionVectorTexture;
        perImageCB["gColor"] = pColorTexture;
        perImageCB["gEmission"] = pEmissionTexture;
        perImageCB["gAlbedo"] = pAlbedoTexture;
        perImageCB["gPositionNormalFwidth"] = pPositionNormalFwidthTexture;
        perImageCB["gPrevIllum"] = mpFilteredPastFbo->getColorTexture(0);
        perImageCB["gPrevMoments"] = mpPrevReprojFbo->getColorTexture(1);
        perImageCB["gLinearZAndNormal"] = mpLinearZAndNormalFbo->getColorTexture(0);
        perImageCB["gPrevLinearZAndNormal"] = pPrevLinearZTexture;
        perImageCB["gPrevHistoryLength"] = mpPrevReprojFbo->getColorTexture(2);

        // Setup variables for our reprojection pass
        perImageCB["gAlpha"] = mAlpha;
        perImageCB["gMomentsAlpha"] = mMomentsAlpha;

        mpReprojection->execute(pRenderContext, mpCurReprojFbo);
    }

    void SVGFPass::computeFilteredMoments(RenderContext* pRenderContext)
    {
        auto perImageCB = mpFilterMoments["PerImageCB"];

        perImageCB["gIllumination"] = mpCurReprojFbo->getColorTexture(0);
        perImageCB["gHistoryLength"] = mpCurReprojFbo->getColorTexture(2);
        perImageCB["gLinearZAndNormal"] = mpLinearZAndNormalFbo->getColorTexture(0);
        perImageCB["gMoments"] = mpCurReprojFbo->getColorTexture(1);

        perImageCB["gPhiColor"] = mPhiColor;
        perImageCB["gPhiNormal"] = mPhiNormal;

        mpFilterMoments->execute(pRenderContext, mpPingPongFbo[0]);
    }

    void SVGFPass::computeAtrousDecomposition(RenderContext* pRenderContext, Texture::SharedPtr pAlbedoTexture)
    {
        auto perImageCB = mpAtrous["PerImageCB"];

        perImageCB["gAlbedo"] = pAlbedoTexture;
        perImageCB["gHistoryLength"] = mpCurReprojFbo->getColorTexture(2);
        perImageCB["gLinearZAndNormal"] = mpLinearZAndNormalFbo->getColorTexture(0);

        perImageCB["gPhiColor"] = mPhiColor;
        perImageCB["gPhiNormal"] = mPhiNormal;

        for (int i = 0; i < mFilterIterations; i++)
        {
            Fbo::SharedPtr curTargetFbo = mpPingPongFbo[1];

            perImageCB["gIllumination"] = mpPingPongFbo[0]->getColorTexture(0);
            perImageCB["gStepSize"] = 1 << i;

            mpAtrous->execute(pRenderContext, curTargetFbo);

            // store the filtered color for the feedback path
            if (i == std::min(mFeedbackTap, mFilterIterations - 1))
            {
                pRenderContext->blit(curTargetFbo->getColorTexture(0)->getSRV(), mpFilteredPastFbo->getRenderTargetView(0));
            }

            std::swap(mpPingPongFbo[0], mpPingPongFbo[1]);
        }

        if (mFeedbackTap < 0)
        {
            pRenderContext->blit(mpCurReprojFbo->getColorTexture(0)->getSRV(), mpFilteredPastFbo->getRenderTargetView(0));
        }
    }
}

