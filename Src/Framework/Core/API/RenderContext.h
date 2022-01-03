#pragma once
#include <stack>
#include <vector>

namespace Desperado
{
    class FullScreenPass;

    /** The rendering context. Use it to bind state and dispatch calls to the GPU
    */
    class dlldecl RenderContext
    {
    public:
        using SharedPtr = std::shared_ptr<RenderContext>;
        using SharedConstPtr = std::shared_ptr<const RenderContext>;

        ~RenderContext();

        /** Create a new render context.
            \return A new object, or throws an exception if creation failed.
        */
        static SharedPtr create();

        /** Clear an FBO.
            \param[in] pFbo The FBO to clear
            \param[in] color The clear color for the bound render-targets
            \param[in] depth The depth clear value
            \param[in] stencil The stencil clear value
            \param[in] flags Optional. Which components of the FBO to clear. By default will clear all attached resource.
            If you'd like to clear a specific color target, you can use RenderContext#clearFboColorTarget().
        */
        void clearFbo(const Fbo* pFbo, const float4& color, float depth, uint8_t stencil, FboAttachmentType flags = FboAttachmentType::All);

        void clearTexture(Texture* pTexture, const float4& clearColor = float4(0, 0, 0, 1));

        /** Ordered draw call.
            \param[in] vertexCount Number of vertices to draw
            \param[in] startVertexLocation The location of the first vertex to read from the vertex buffers (offset in vertices)
        */
        void draw(uint32_t vertexCount, uint32_t startVertexLocation);

        /** Ordered instanced draw call.
            \param[in] vertexCount Number of vertices to draw
            \param[in] instanceCount Number of instances to draw
            \param[in] startVertexLocation The location of the first vertex to read from the vertex buffers (offset in vertices)
            \param[in] startInstanceLocation A value which is added to each index before reading per-instance data from the vertex buffer
        */
        void drawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation);

    private:
        RenderContext();
        bool applyGraphicsVars();
        bool prepareForDraw();
    };
}
