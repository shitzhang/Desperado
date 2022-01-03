#pragma once
#include "RenderPass.h"
#include "FullScreenPass.h"
#include "Core/API/FBO.h"

namespace Desperado {

    class dlldecl SVGFPass : public RenderPass
    {
    public:
        using SharedPtr = std::shared_ptr<SVGFPass>;

        static SharedPtr create(const InternalDictionary& dict = {});

        virtual std::string getDesc() override { return "SVGF Denoising Pass"; }
        virtual void execute(const RenderData& renderData) override;

        Fbo::SharedPtr mpPingPongFbo[2];
        Fbo::SharedPtr mpLinearZAndNormalFbo;
        Fbo::SharedPtr mpFilteredPastFbo;
        Fbo::SharedPtr mpCurReprojFbo;
        Fbo::SharedPtr mpPrevReprojFbo;
        Fbo::SharedPtr mpFilteredIlluminationFbo;
        Fbo::SharedPtr mpFinalFbo;
    private:
        SVGFPass(const InternalDictionary& dict);

        bool init(const InternalDictionary& dict);
        void allocateFbos(uint2 dim);
        void clearBuffers(const RenderData& renderData);

        void computeLinearZAndNormal(Texture::SharedPtr pLinearZTexture,
            Texture::SharedPtr pWorldNormalTexture);
        void computeReprojection(Texture::SharedPtr pAlbedoTexture,
            Texture::SharedPtr pColorTexture, Texture::SharedPtr pEmissionTexture,
            Texture::SharedPtr pMotionVectorTexture,
            Texture::SharedPtr pPositionNormalFwidthTexture,
            Texture::SharedPtr pPrevLinearZAndNormalTexture);
        void computeFilteredMoments();
        void computeAtrousDecomposition(Texture::SharedPtr pAlbedoTexture);

        bool mBuffersNeedClear = false;

        // SVGF parameters
        bool    mFilterEnabled = true;
        int     mFilterIterations = 4;
        int     mFeedbackTap = 1;
        float   mVarainceEpsilon = 1e-4f;
        float   mPhiColor = 10.0f;
        float   mPhiNormal = 128.0f;
        float   mAlpha = 0.05f;
        float   mMomentsAlpha = 0.2f;

        // SVGF passes
        FullScreenPass::SharedPtr mpPackLinearZAndNormal;
        FullScreenPass::SharedPtr mpReprojection;
        FullScreenPass::SharedPtr mpFilterMoments;
        FullScreenPass::SharedPtr mpAtrous;
        FullScreenPass::SharedPtr mpFinalModulate;

        // Intermediate framebuffers
        //Fbo::SharedPtr mpPingPongFbo[2];
        //Fbo::SharedPtr mpLinearZAndNormalFbo;
        //Fbo::SharedPtr mpFilteredPastFbo;
        //Fbo::SharedPtr mpCurReprojFbo;
        //Fbo::SharedPtr mpPrevReprojFbo;
        //Fbo::SharedPtr mpFilteredIlluminationFbo;
        //Fbo::SharedPtr mpFinalFbo;

        Shader::SharedPtr mpPackLinearZAndNormalShader;
        Shader::SharedPtr mpReprojectionShader;
        Shader::SharedPtr mpFilterMomentsShader;
        Shader::SharedPtr mpAtrousShader;
        Shader::SharedPtr mpFinalModulateShader;

        Texture::SharedPtr mpPrevLinearZAndNormalTexture;
    };
}