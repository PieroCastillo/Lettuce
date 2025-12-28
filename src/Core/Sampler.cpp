// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

Sampler Device::CreateSampler(const SamplerDesc& desc)
{
    VkSamplerCreateInfo samplerCI = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    };

    VkSampler sampler;
    handleResult(vkCreateSampler(impl->m_device, &samplerCI, nullptr, &sampler));
    return impl->samplers.allocate(std::move(sampler));
}

void Device::Destroy(Sampler sampler)
{
    vkDestroySampler(impl->m_device, impl->samplers.get(sampler), nullptr);
    impl->samplers.free(sampler);
}