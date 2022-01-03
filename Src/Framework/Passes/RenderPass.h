#pragma once

#include "Utils/InternalDictionary.h"
#include "utils/Math/Vector.h"
#include "Core/API/Texture.h"
#include "scene.h"

namespace Desperado
{
    class dlldecl RenderData
    {
    public:
        InternalDictionary::Value& operator[](const std::string& name) const { return getResource(name); }
        InternalDictionary::Value& getResource(const std::string& name) const { return (*mpDictionary)[name]; }
        InternalDictionary& getDictionary() const { return (*mpDictionary); }
        InternalDictionary::SharedPtr getDictionaryPtr() const { return mpDictionary; }

        RenderData(const std::string& passName, const InternalDictionary::SharedPtr& pDict);
    protected:
        const std::string& mName;
        InternalDictionary::SharedPtr mpDictionary;
    };


    class dlldecl RenderPass : public std::enable_shared_from_this<RenderPass>
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

