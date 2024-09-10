//
// Created by piero on 08/09/2024.
//
#include "Lettuce/Core/common.hpp"
#include <vector>
#include <functional>
#include <string>
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/TextureView.hpp"
#include "Lettuce/Core/Sampler.hpp"
#include "Lettuce/Core/RenderPass.hpp"
#include "Lettuce/Foundation/Attachment.hpp"
#include "Lettuce/Foundation/RenderGroup.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundation;

RenderGroup::RenderGroup(std::string name)
{
    if (!released)
    {
        throw std::runtime_error("renderGroup with name: " + name + " didn't have been released!");
    }
    _name = name;
}
void RenderGroup::AddAttachment(Attachment attachment)
{
    attachments.push_back(attachment);
}
void RenderGroup::AddResource(Resource resource, ResourceUse use)
{
    resources.push_back(resource);
    uses.push_back(use);
}
void RenderGroup::Release()
{
    attachments.clear();
    resources.clear();
    uses.clear();
    released = true;
}