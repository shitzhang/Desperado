
#pragma once


#include "Utils/InternalDictionary.h"
#include "utils/Math/Vector.h"
//#include "Core/API/Texture.h"
#include "scene.h"
#include "FBO.h"

namespace Desperado
{
    class RenderData
    {
    public:

        InternalDictionary& getDictionary() const { return (*mpDictionary); }

        InternalDictionary::SharedPtr getDictionaryPtr() const { return mpDictionary; }

        const uint2& getDefaultTextureDims() const { return mDefaultTexDims; }

        //ResourceFormat getDefaultTextureFormat() const { return mDefaultTexFormat; }
    protected:
        RenderData(const std::string& passName, const InternalDictionary::SharedPtr& pDict, const uint2& defaultTexDims);
        const std::string& mName;

        InternalDictionary::SharedPtr mpDictionary;
        uint2 mDefaultTexDims;
        //ResourceFormat mDefaultTexFormat;
    };


    class RenderPass : public std::enable_shared_from_this<RenderPass>
    {
    public:
        using SharedPtr = std::shared_ptr<RenderPass>;
        virtual ~RenderPass() = default;

        virtual void execute(const RenderData& renderData) = 0; 

        virtual std::string getDesc() = 0;

        virtual void setScene(const std::shared_ptr<Scene>& pScene) {}

        const std::string& getName() const { return mName; }

    protected:
        RenderPass() = default;
        std::string mName;
        //std::function<void(void)> mPassChangedCB = [] {};
    };
}

