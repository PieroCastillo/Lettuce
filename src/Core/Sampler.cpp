// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/Sampler.hpp"

using namespace Lettuce::Core;

LettuceResult Sampler::Create(const std::weak_ptr<IDevice>& device, const SamplerCreateInfo& createInfo) 
{
    m_device = device->m_device;

    VkSamplerCreateInfo samplerCI = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    };
    auto result = vkCreateSampler(m_device, &samplerCI, nullptr, &m_sampler);

    return LettuceResult::Success;
}

void Sampler::Release() 
{
    vkDestroySampler(m_device, m_sampler, nullptr);
}