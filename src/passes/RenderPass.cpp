#include "stdafx.h"
#include "RenderPass.h"

namespace Desperado
{
    RenderData::RenderData(const std::string& passName, const InternalDictionary::SharedPtr& pDict)
        : mName(passName)
        , mpDictionary(pDict)
    {
        if (!mpDictionary) mpDictionary = InternalDictionary::create();
    }
}
