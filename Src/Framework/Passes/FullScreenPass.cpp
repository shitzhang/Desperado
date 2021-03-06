#include "stdafx.h"
#include "FullScreenPass.h"

namespace Desperado
{
    namespace
    {
        struct FullScreenPassData
        {
            uint32_t quadVBO;
            uint32_t quadVAO = 0;;
            uint64_t objectCount = 0;
        };

        FullScreenPassData gFullScreenData;

        float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
        };

        void initFullScreenData(uint32_t& quadVBO, uint32_t& quadVAO)
        {          
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            assert(quadVAO);
        }
    }

    FullScreenPass::FullScreenPass()
    {
        gFullScreenData.objectCount++;

        if (gFullScreenData.quadVAO == 0)
        {
            initFullScreenData(gFullScreenData.quadVBO, gFullScreenData.quadVAO);
        }
        assert(gFullScreenData.quadVAO);
        glBindVertexArray(gFullScreenData.quadVAO);
    }

    FullScreenPass::~FullScreenPass()
    {
        assert(gFullScreenData.objectCount > 0);

        gFullScreenData.objectCount--;
        if (gFullScreenData.objectCount == 0)
        {
            glDeleteBuffers(1, &gFullScreenData.quadVBO);
            glDeleteVertexArrays(1, &gFullScreenData.quadVAO);
        }
    }

    FullScreenPass::SharedPtr FullScreenPass::create()
    {
        return SharedPtr(new FullScreenPass());
    }

    void FullScreenPass::execute(const Fbo::SharedPtr& pFbo) const
    {
        if (pFbo == nullptr) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        else {
            pFbo->bind();
        }
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);             // disable depth test so screen-space quad isn't discarded due to depth test.
      
        glBindVertexArray(gFullScreenData.quadVAO);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        Fbo::unBind();
        glBindVertexArray(0);
    }
}
