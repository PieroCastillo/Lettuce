// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

auto Device::CreateShader(const ShaderBinaryDesc& desc) -> ShaderBinary
{
    VkShaderModuleCreateInfo shaderCI = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = desc.bytecode.size() * sizeof(uint32_t),
        .pCode = desc.bytecode.data(),
    };
    VkShaderModule shaderModule;
    handleResult(vkCreateShaderModule(impl->m_device, &shaderCI, nullptr, &shaderModule));

    return impl->shaders.allocate(std::move(shaderModule));
}

auto Device::Destroy(ShaderBinary shader)
{
    vkDestroyShaderModule(impl->m_device, impl->shaders.get(shader), nullptr);
    impl->shaders.free(shader);
}