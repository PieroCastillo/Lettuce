/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_SAMPLER_HPP
#define LETTUCE_CORE_SAMPLER_HPP

// standard headers
#include <memory>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct SamplerCreateInfo
    {
        VkFilter magFilter;
        VkFilter minFilter;
        VkSamplerAddressMode addressModeU;
        VkSamplerAddressMode addressModeV;
        VkSamplerAddressMode addressModeW;
        float mipLodBias;
        bool anisotropyEnable;
        float maxAnisotropy;
        bool compareEnable;
        VkCompareOp compareOp;
        float minLod;
        float maxLod;
        VkBorderColor borderColor;
    };

    class Sampler
    {
    private:
    public:
        VkDevice m_device;
        VkSampler m_sampler;

        void Create(const IDevice& device, const SamplerCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_SAMPLER_HPP