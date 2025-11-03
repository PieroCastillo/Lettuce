// standard headers
#include <print>

// external headers
#include "spirv_reflect.h"

// project headers
#include "Lettuce/Core/ShaderPack.hpp"

using namespace Lettuce::Core;

VkDescriptorType mapReflectTypeToVk(SpvReflectDescriptorType rtype) {
    switch (rtype) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:              return VK_DESCRIPTOR_TYPE_SAMPLER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:       return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:       return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:     return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

const std::string mapReflectTypeToString(SpvReflectDescriptorType rtype) {
    switch (rtype) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:              return "Sampler";
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "ImageSampler";
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:        return "SampledImage";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:        return "StorageImage";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return "UniformTexelBUuffer";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return "StorageTexelBUuffer";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:       return "UniformBuffer";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:       return "StorageBuffer";
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "UniformBufferDynamic";
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "StorageBufferDynamic";
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:     return "InputAttachment";
    default: return "MaxEnum";
    }
}

inline void handleSpvResult(SpvReflectResult res)
{
    if (res != SPV_REFLECT_RESULT_SUCCESS) [[unlikely]]
    {
        throw LettuceException(LettuceResult::ShaderReflectionFailed);
    }
}

void ShaderPack::Create(const IDevice& device, const ShaderPackCreateInfo& createInfo)
{
    m_device = device.m_device;

    SpvReflectShaderModule mod;
    handleSpvResult(spvReflectCreateShaderModule(createInfo.shaderByteData.size() * sizeof(uint32_t), createInfo.shaderByteData.data(), &mod));

    uint32_t setCount;
    handleSpvResult(spvReflectEnumerateDescriptorSets(&mod, &setCount, nullptr));
    std::vector<SpvReflectDescriptorSet*> setsRefl(setCount);
    handleSpvResult(spvReflectEnumerateDescriptorSets(&mod, &setCount, setsRefl.data()));

    m_descriptorsInfo.reserve(setCount);

    // create descriptors set layouts
    // unordered map with name-binding is located in descriptor table
    for (const auto* set : setsRefl)
    {
        DescriptorBindingsInfo bindingsInfo;
        bindingsInfo.setId = set->set;
        for (int i = 0; i < set->binding_count; ++i)
        {
            auto* binding = set->bindings[i];
            bindingsInfo.bindingId.push_back(binding->binding);
            bindingsInfo.counts.push_back(binding->count);
            bindingsInfo.names.emplace_back(binding->name);
            bindingsInfo.types.push_back(mapReflectTypeToVk(binding->descriptor_type));

            std::println("set: {}, binding: {}, name: {}, type: {}", set->set,
                binding->binding,
                binding->name,
                mapReflectTypeToString(binding->descriptor_type));
        }
        m_descriptorsInfo.push_back(std::move(bindingsInfo));
    }

    VkShaderModuleCreateInfo shaderCI = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = createInfo.shaderByteData.size() * sizeof(uint32_t),
        .pCode = createInfo.shaderByteData.data(),
    };
    handleResult(vkCreateShaderModule(m_device, &shaderCI, nullptr, &m_shaderModule));

    spvReflectDestroyShaderModule(&mod);
}

void ShaderPack::Release()
{
    vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
}

auto ShaderPack::GetDescriptorsInfo() -> std::vector<DescriptorBindingsInfo>
{
    return m_descriptorsInfo;
}