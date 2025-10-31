/*
Created by @PieroCastillo on 2025-10-29
*/
#ifndef LETTUCE_CORE_SHADER_PACK
#define LETTUCE_CORE_SHADER_PACK

// standard headers
#include <vector>
#include <cstdint>
#include <string>

// project headers
#include "common.hpp"

namespace Lettuce::Core
{
    struct DescriptorBindingsInfo {
        uint32_t setId;
        std::vector<uint32_t> bindingId;
        std::vector<std::string> names;
        std::vector<uint32_t> counts;
        std::vector<VkDescriptorType> types;
    };

    struct ShaderPackCreateInfo
    {
        std::vector<uint32_t> shaderByteData;
    };

    class ShaderPack
    {
        std::vector<DescriptorBindingsInfo> m_descriptorsInfo;
    public:
        VkDevice m_device;
        VkShaderModule m_shaderModule;
        void Create(const IDevice& device, const ShaderPackCreateInfo& createInfo);
        void Release();

        auto GetDescriptorsInfo() -> std::vector<DescriptorBindingsInfo>;
    };
};

#endif // LETTUCE_CORE_SHADER_PACK