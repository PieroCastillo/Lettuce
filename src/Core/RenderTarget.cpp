// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/RenderTarget.hpp"

using namespace Lettuce::Core;

void RenderTarget::Create(const IDevice& device, const RenderTargetCreateInfo& createInfo)
{

}

void RenderTarget::Release()
{

}

VkFormat RenderTarget::GetFormat()
{
    return m_format;
}