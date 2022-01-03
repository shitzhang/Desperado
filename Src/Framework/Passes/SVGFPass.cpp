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
        const char kPackLinearZAndNormalShader[] = "Shader/SVGFPackLinearZAndNormal.fs";
        const char kReprojectShader[] = "Shader/SVGFReproject.fs";
        const char kAtrousShader[] = "Shader/SVGFAtrous.fs";
        const char kFilterMomentShader[] = "Shader/SVGFFilterMoments.fs";
        const char kFinalModulateShader[] = "Shader/SVGFFinalModulate.fs";

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
        const char kInputBufferAlbedo[] = "gAlbedo";
        const char kInputBufferEmission[] = "gEmission";
        const char kInputBufferWorldPosition[] = "gPosition";
        const char kInputBufferWorldNormal[] = "gNormal";
        const char kInputBufferPosNormalFwidth[] = "gPositionNormalFwidth";
        const char kInputBufferLinearZ[] = "gLinearZ";
        const char kInputBufferMotionVector[] = "gMotion";
        const char kInputBufferMeshId[] = "gMeshId";

        const char kInputBufferColor[] = "color";
        const char kInputBufferDirectColor[] = "directColor";
        const char kInputBufferIndirectColor[] = "indirectColor";

        // Internal buffer names
        const char kInternalBufferPreviousLinearZAndNormal[] = "Previous Linear Z and Packed Normal";
        const char kInternalBufferPreviousLighting[] = "Previous Lighting";
        const char kInternalBufferPreviousMoments[] = "Previous Moments";

        // Output buffer name
        const char kOutputBufferFilteredImage[] = "output";
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

        mpPackLinearZAndNormalShader = Shader::create("Shader/quad.vs", kPackLinearZAndNormalShader);
        mpReprojectionShader = Shader::create("Shader/quad.vs", kReprojectShader);
        mpAtrousShader = Shader::create("Shader/quad.vs", kAtrousShader);
        mpFilterMomentsShader = Shader::create("Shader/quad.vs", kFilterMomentShader);
        mpFinalModulateShader = Shader::create("Shader/quad.vs", kFinalModulateShader);
       

        mpPackLinearZAndNormal = FullScreenPass::create();
        mpReprojection = FullScreenPass::create();
        mpAtrous = FullScreenPass::create();
        mpFilterMoments = FullScreenPass::create();
        mpFinalModulate = FullScreenPass::create();
        assert(mpPackLinearZAndNormal && mpReprojection && mpAtrous && mpFilterMoments && mpFinalModulate);

        allocateFbos(uint2(SCR_WIDTH, SCR_HEIGHT));
        mBuffersNeedClear = true;

        mpPrevLinearZAndNormalTexture = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, 0);
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
        Texture::SharedPtr pAlbedoTexture = renderData[kInputBufferAlbedo];
        Texture::SharedPtr pColorTexture = renderData[kInputBufferColor];
        Texture::SharedPtr pEmissionTexture = renderData[kInputBufferEmission];
        Texture::SharedPtr pWorldPositionTexture = renderData[kInputBufferWorldPosition];
        Texture::SharedPtr pWorldNormalTexture = renderData[kInputBufferWorldNormal];
        Texture::SharedPtr pPosNormalFwidthTexture = renderData[kInputBufferPosNormalFwidth];
        Texture::SharedPtr pLinearZTexture = renderData[kInputBufferLinearZ];
        Texture::SharedPtr pMotionVectorTexture = renderData[kInputBufferMotionVector];

        Texture::SharedPtr pOutputTexture = renderData[kOutputBufferFilteredImage];

        assert(mpFilteredIlluminationFbo &&
            mpFilteredIlluminationFbo->getWidth() == pAlbedoTexture->getWidth() &&
            mpFilteredIlluminationFbo->getHeight() == pAlbedoTexture->getHeight());

        if (mBuffersNeedClear)
        {
            //clearBuffers(pRenderContext, renderData);
            //mBuffersNeedClear = false;
        }

        if (mFilterEnabled)
        {
            // Grab linear z and its derivative and also pack the normal into
            // the last two channels of the mpLinearZAndNormalFbo.
            computeLinearZAndNormal(pLinearZTexture, pWorldNormalTexture);

            // Demodulate input color & albedo to get illumination and lerp in
            // reprojected filtered illumination from the previous frame.
            // Stores the result as well as initial moments and an updated
            // per-pixel history length in mpCurReprojFbo.

            /*Texture::SharedPtr pPrevLinearZAndNormalTexture =
                renderData[kInternalBufferPreviousLinearZAndNormal];*/
            //mpBlitFbo->blit(mpLinearZAndNormalFbo->getColorTexture(0), mpPrevLinearZAndNormalTexture);
            computeReprojection(pAlbedoTexture, pColorTexture, pEmissionTexture,
                pMotionVectorTexture, pPosNormalFwidthTexture,
                mpPrevLinearZAndNormalTexture);

            // Do a first cross-bilateral filtering of the illumination and
            // estimate its variance, storing the result into a float4 in
            // mpPingPongFbo[0].  Takes mpCurReprojFbo as input.
            computeFilteredMoments();
            //Fbo::blit(mpPingPongFbo[0], 0, pOutputTexture);

            // Filter illumination from mpCurReprojFbo[0], storing the result
            // in mpPingPongFbo[0].  Along the way (or at the end, depending on
            // the value of mFeedbackTap), save the filtered illumination for
            // next time into mpFilteredPastFbo.
            
            computeAtrousDecomposition(pAlbedoTexture);

            // Compute albedo * filtered illumination and add emission back in.
            
            //auto perImageCB = mpFinalModulate["PerImageCB"];
            //perImageCB["gAlbedo"] = pAlbedoTexture;
            //perImageCB["gEmission"] = pEmissionTexture;
            //perImageCB["gIllumination"] = mpPingPongFbo[0]->getColorTexture(0);

            mpFinalModulateShader->use();
            mpFinalModulateShader->setTexture2D("gAlbedo", pAlbedoTexture);
            mpFinalModulateShader->setTexture2D("gEmission", pEmissionTexture);
            mpFinalModulateShader->setTexture2D("gIllumination", mpPingPongFbo[0]->getColorTexture(0));

            mpFinalModulate->execute(mpFinalFbo);

            // Blit into the output texture.
            Fbo::blit(mpFinalFbo, 0, pOutputTexture);
            
            // Swap resources so we're ready for next frame.
            std::swap(mpCurReprojFbo, mpPrevReprojFbo);
            Fbo::blit(mpLinearZAndNormalFbo, 0, mpPrevLinearZAndNormalTexture);
        }
        else
        {
            Fbo::blit(pColorTexture, pOutputTexture);
        }
    }

    bool SVGFPass::init(const InternalDictionary& dict)
    {
        return false;
    }

    void SVGFPass::allocateFbos(uint2 dim)
    {
        {
            // Screen-size FBOs with 3 MRTs: one that is RGBA32F, one that is
            // RG32F for the luminance moments, and one that is R16F.
            Fbo::Desc desc;
            desc.setColorTarget(0, GL_RGBA32F, GL_RGBA); // illumination
            desc.setColorTarget(1, GL_RG32F, GL_RG);   // moments
            desc.setColorTarget(2, GL_R32F, GL_RED);    // history length
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
    }

    void SVGFPass::clearBuffers(const RenderData& renderData)
    {
        //pRenderContext->clearFbo(mpPingPongFbo[0].get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        //pRenderContext->clearFbo(mpPingPongFbo[1].get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        //pRenderContext->clearFbo(mpLinearZAndNormalFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        //pRenderContext->clearFbo(mpFilteredPastFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        //pRenderContext->clearFbo(mpCurReprojFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        //pRenderContext->clearFbo(mpPrevReprojFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);
        //pRenderContext->clearFbo(mpFilteredIlluminationFbo.get(), float4(0), 1.0f, 0, FboAttachmentType::All);

        //pRenderContext->clearTexture(renderData[kInternalBufferPreviousLinearZAndNormal]->asTexture().get());
        //pRenderContext->clearTexture(renderData[kInternalBufferPreviousLighting]->asTexture().get());
        //pRenderContext->clearTexture(renderData[kInternalBufferPreviousMoments]->asTexture().get());
    }

    // Extracts linear z and its derivative from the linear Z texture and packs
    // the normal from the world normal texture and packes them into the FBO.
    // (It's slightly wasteful to copy linear z here, but having this all
    // together in a single buffer is a small simplification, since we make a
    // copy of it to refer to in the next frame.)
    void SVGFPass::computeLinearZAndNormal(Texture::SharedPtr pLinearZTexture,
        Texture::SharedPtr pWorldNormalTexture)
    {
        /*auto perImageCB = mpPackLinearZAndNormal["PerImageCB"];
        perImageCB["gLinearZ"] = pLinearZTexture;
        perImageCB["gNormal"] = pWorldNormalTexture;*/
        mpPackLinearZAndNormalShader->use();
        mpPackLinearZAndNormalShader->setTexture2D("gLinearZ", pLinearZTexture);
        mpPackLinearZAndNormalShader->setTexture2D("gNormal", pWorldNormalTexture);
        mpPackLinearZAndNormal->execute(mpLinearZAndNormalFbo);
    }

    void SVGFPass::computeReprojection(Texture::SharedPtr pAlbedoTexture,
        Texture::SharedPtr pColorTexture, Texture::SharedPtr pEmissionTexture,
        Texture::SharedPtr pMotionVectorTexture,
        Texture::SharedPtr pPositionNormalFwidthTexture,
        Texture::SharedPtr pPrevLinearZTexture)
    {
        //auto perImageCB = mpReprojection["PerImageCB"];       
        //perImageCB["gMotion"] = pMotionVectorTexture;
        //perImageCB["gColor"] = pColorTexture;
        //perImageCB["gEmission"] = pEmissionTexture;
        //perImageCB["gAlbedo"] = pAlbedoTexture;
        //perImageCB["gPositionNormalFwidth"] = pPositionNormalFwidthTexture;
        //perImageCB["gPrevIllum"] = mpFilteredPastFbo->getColorTexture(0);
        //perImageCB["gPrevMoments"] = mpPrevReprojFbo->getColorTexture(1);
        //perImageCB["gLinearZAndNormal"] = mpLinearZAndNormalFbo->getColorTexture(0);
        //perImageCB["gPrevLinearZAndNormal"] = pPrevLinearZTexture;
        //perImageCB["gPrevHistoryLength"] = mpPrevReprojFbo->getColorTexture(2);

        //perImageCB["gAlpha"] = mAlpha;
        //perImageCB["gMomentsAlpha"] = mMomentsAlpha;

        mpReprojectionShader->use();
        mpReprojectionShader->setTexture2D("gMotion", pMotionVectorTexture);
        mpReprojectionShader->setTexture2D("gColor", pColorTexture);
        mpReprojectionShader->setTexture2D("gEmission", pEmissionTexture);
        mpReprojectionShader->setTexture2D("gAlbedo", pAlbedoTexture);
        mpReprojectionShader->setTexture2D("gPositionNormalFwidth", pPositionNormalFwidthTexture);
        mpReprojectionShader->setTexture2D("gPrevIllum", mpFilteredPastFbo->getColorTexture(0));
        mpReprojectionShader->setTexture2D("gPrevMoments", mpPrevReprojFbo->getColorTexture(1));
        mpReprojectionShader->setTexture2D("gLinearZAndNormal", mpLinearZAndNormalFbo->getColorTexture(0));
        mpReprojectionShader->setTexture2D("gPrevLinearZAndNormal", pPrevLinearZTexture);
        mpReprojectionShader->setTexture2D("gPrevHistoryLength", mpPrevReprojFbo->getColorTexture(2));

        mpReprojectionShader->setFloat("gAlpha", mAlpha);
        mpReprojectionShader->setFloat("gMomentsAlpha", mMomentsAlpha);

        mpReprojection->execute(mpCurReprojFbo);
    }

    void SVGFPass::computeFilteredMoments()
    {
        /*auto perImageCB = mpFilterMoments["PerImageCB"];

        perImageCB["gIllumination"] = mpCurReprojFbo->getColorTexture(0);
        perImageCB["gHistoryLength"] = mpCurReprojFbo->getColorTexture(2);
        perImageCB["gLinearZAndNormal"] = mpLinearZAndNormalFbo->getColorTexture(0);
        perImageCB["gMoments"] = mpCurReprojFbo->getColorTexture(1);

        perImageCB["gPhiColor"] = mPhiColor;
        perImageCB["gPhiNormal"] = mPhiNormal;*/

        mpFilterMomentsShader->use();
        mpFilterMomentsShader->setTexture2D("gIllumination", mpCurReprojFbo->getColorTexture(0));
        mpFilterMomentsShader->setTexture2D("gHistoryLength", mpCurReprojFbo->getColorTexture(2));
        mpFilterMomentsShader->setTexture2D("gLinearZAndNormal", mpLinearZAndNormalFbo->getColorTexture(0));
        mpFilterMomentsShader->setTexture2D("gMoments", mpCurReprojFbo->getColorTexture(1));

        mpFilterMomentsShader->setFloat("gPhiColor", mPhiColor);
        mpFilterMomentsShader->setFloat("gPhiNormal", mPhiNormal);

        mpFilterMoments->execute(mpPingPongFbo[0]);
    }

    void SVGFPass::computeAtrousDecomposition(Texture::SharedPtr pAlbedoTexture)
    {
        //auto perImageCB = mpAtrous["PerImageCB"];

        //perImageCB["gAlbedo"] = pAlbedoTexture;
        //perImageCB["gHistoryLength"] = mpCurReprojFbo->getColorTexture(2);
        //perImageCB["gLinearZAndNormal"] = mpLinearZAndNormalFbo->getColorTexture(0);

        //perImageCB["gPhiColor"] = mPhiColor;
        //perImageCB["gPhiNormal"] = mPhiNormal;

        mpAtrousShader->use();
        mpAtrousShader->setTexture2D("gAlbedo", pAlbedoTexture);
        mpAtrousShader->setTexture2D("gHistoryLength", mpCurReprojFbo->getColorTexture(2));
        mpAtrousShader->setTexture2D("gLinearZAndNormal", mpLinearZAndNormalFbo->getColorTexture(0));

        mpAtrousShader->setFloat("gPhiColor", mPhiColor);
        mpAtrousShader->setFloat("gPhiNormal", mPhiNormal);


        for (int i = 0; i < mFilterIterations; i++)
        {
            Fbo::SharedPtr curTargetFbo = mpPingPongFbo[1];

            //perImageCB["gIllumination"] = mpPingPongFbo[0]->getColorTexture(0);
            //perImageCB["gStepSize"] = 1 << i;

            mpAtrousShader->setTexture2D("gIllumination", mpPingPongFbo[0]->getColorTexture(0));
            mpAtrousShader->setInt("gStepSize", 1 << i);

            mpAtrous->execute(curTargetFbo);

            // store the filtered color for the feedback path
            if (i == std::min(mFeedbackTap, mFilterIterations - 1))
            {               
                Fbo::blit(curTargetFbo, 0, mpFilteredPastFbo, 0);
            }

            //swap shared_ptr
            std::swap(mpPingPongFbo[0], mpPingPongFbo[1]);
        }

        if (mFeedbackTap < 0)
        {           
            Fbo::blit(mpCurReprojFbo, 0, mpFilteredPastFbo, 0);
        }
    }
}

