#pragma once

#include "Passes/RenderPass.h"
#include "Core/API/FBO.h"
//#include "shader.h"

namespace Desperado
{
    class FullScreenPass : public std::enable_shared_from_this<FullScreenPass>
    {
    public:
        using SharedPtr = std::shared_ptr<FullScreenPass>;

        virtual ~FullScreenPass();


        static SharedPtr create();

        virtual void execute(const Fbo::SharedPtr& pFbo = nullptr) const;

        //std::shared_ptr<Shader> getShaderPtr() { return mpShader; }

    protected:
        FullScreenPass();
    private:
        //std::shared_ptr<Shader> mpShader;
    };
}

