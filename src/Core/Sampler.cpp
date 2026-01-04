// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

Sampler Device::CreateSampler(const SamplerDesc& desc)
{
    VkSamplerCreateInfo samplerCI = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = ToVkFilter(desc.magFilter),
        .minFilter = ToVkFilter(desc.minFilter),
        .mipmapMode = ToVkMipmapMode(desc.mipmap),
        .addressModeU = ToVkAddressMode(desc.addressModeU),
        .addressModeV = ToVkAddressMode(desc.addressModeV),
        .addressModeW = ToVkAddressMode(desc.addressModeW),
        .mipLodBias = 0,
        .anisotropyEnable = desc.anisotropy > 1.0f,
        .maxAnisotropy = desc.anisotropy > 1.0f ? std::min(desc.anisotropy, impl->props.maxSamplerAnisotropy) : 1.0f,
        .compareEnable = desc.depthCompare,
        .compareOp = desc.depthCompare ? VK_COMPARE_OP_LESS_OR_EQUAL : VK_COMPARE_OP_ALWAYS,
        .minLod = 0,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
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