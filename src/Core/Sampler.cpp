// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Sampler.hpp"

using namespace Lettuce::Core;

void Sampler::Create(const std::weak_ptr<IDevice>& device, const SamplerCreateInfo& createInfo) 
{
    m_device = (device.lock())->m_device;

    VkSamplerCreateInfo samplerCI = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    };
    
    handleResult(vkCreateSampler(m_device, &samplerCI, nullptr, &m_sampler));
}

void Sampler::Release() 
{
    vkDestroySampler(m_device, m_sampler, nullptr);
}