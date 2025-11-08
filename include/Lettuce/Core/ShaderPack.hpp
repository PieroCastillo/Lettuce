/*
Created by @PieroCastillo on 2025-10-29
*/
#ifndef LETTUCE_CORE_SHADER_PACK
#define LETTUCE_CORE_SHADER_PACK

// standard headers
#include <cstdint>
#include <span>
#include <string>
#include <vector>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct ShaderPackCreateInfo
    {
        std::span<const uint32_t> shaderByteData;
    };

    class ShaderPack
    {
        std::vector<DescriptorSetLayoutInfo> m_descriptorsInfo;
    public:
        VkDevice m_device;
        VkShaderModule m_shaderModule;
        void Create(const IDevice& device, const ShaderPackCreateInfo& createInfo);
        void Release();

        auto GetDescriptorsInfo() -> std::vector<DescriptorSetLayoutInfo>;
    };
};

#endif // LETTUCE_CORE_SHADER_PACK