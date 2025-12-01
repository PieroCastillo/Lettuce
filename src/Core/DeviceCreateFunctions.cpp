// standard headers
#include <array>
#include <expected>
#include <iostream>
#include <memory>
#include <print>
#include <variant>
#include <vector>

// project headers
#include "Lettuce/Core/Device.hpp"

// external headers
#include <ktx.h>
#define VK_NO_PROTOTYPE
#include <ktxvulkan.h>

using namespace Lettuce::Core;

auto Device::CreateSwapchain(const SwapchainCreateInfo& createInfo) -> Result<Swapchain>
{
    try
    {
        auto swapchain = std::make_shared<Swapchain>();
        swapchain->Create(*this, createInfo);
        return swapchain;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) -> Result<DescriptorTable>
{
    try
    {
        auto dt = std::make_shared<DescriptorTable>();
        dt->Create(*this, createInfo);
        return dt;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateContext(const DeviceExecutionContextCreateInfo& createInfo) -> Result<DeviceExecutionContext>
{
    try
    {
        auto ctx = std::make_shared<DeviceExecutionContext>();
        ctx->Create(*this, createInfo);
        return ctx;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreatePipeline(const ComputePipelineCreateData& data) -> Result<Pipeline>
{
    try
    {
        auto pipeline = std::make_shared<Pipeline>();

        ComputePipelineCreateInfo cpipelineCI = {
            .entryPoint = data.computeEntryPoint,
            .shaderModule = (data.shaders.lock())->m_shaderModule,
            .layout = (data.descriptorTable.lock())->m_pipelineLayout,
        };
        pipeline->Create(*this, cpipelineCI);
        return pipeline;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreatePipeline(const GraphicsPipelineCreateData& data) -> Result<Pipeline>
{
    try
    {
        auto pipeline = std::make_shared<Pipeline>();

        std::vector<VkFormat> colorFormats;
        colorFormats.reserve(data.colorTargets.size());
        for (const auto& target : data.colorTargets)
        {
            colorFormats.push_back(target.lock()->GetFormat());
        }

        GraphicsPipelineCreateInfo gpipelineCI = {
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .fragmentShadingRate = false, // TODO: impl fragment shading rate
            .colorAttachmentFormats = colorFormats,
            .layout = (data.descriptorTable.lock())->m_pipelineLayout,
        };
        // create vertex input & attributes
        uint32_t idx = 0;
        uint32_t elementSize = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
        for (const auto input : data.inputs)
        {
            switch (input)
            {
            case VertexInput::float32x2:
                elementSize = sizeof(float) * 2;
                format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case VertexInput::float32x3:
                elementSize = sizeof(float) * 3;
                format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case VertexInput::float32x4:
                elementSize = sizeof(float) * 4;
                format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            }
            gpipelineCI.vertexAttributes.push_back({ idx, idx, format, 0 });
            gpipelineCI.vertexBindings.push_back({ idx, elementSize, VK_VERTEX_INPUT_RATE_VERTEX });
            ++idx;
        }

        // create shader module
        auto shaderModule = data.shaders.lock()->m_shaderModule;

        // define shaders for the pipeline
        if (data.fragmentEntryPoint.empty())
        {
            throw LettuceException(LettuceResult::InvalidShaderEntryPoint);
        }
        gpipelineCI.entryPoints.push_back(data.fragmentEntryPoint);
        gpipelineCI.shaderModules.push_back(shaderModule);
        gpipelineCI.stages.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);

        if (data.meshEntryPoint.has_value())
        {
            gpipelineCI.useMeshShader = true;
            gpipelineCI.entryPoints.push_back(data.meshEntryPoint.value());
            gpipelineCI.shaderModules.push_back(shaderModule);
            gpipelineCI.stages.push_back(VK_SHADER_STAGE_MESH_BIT_EXT);

            if (data.taskEntryPoint.has_value())
            {
                gpipelineCI.entryPoints.push_back(data.taskEntryPoint.value());
                gpipelineCI.shaderModules.push_back(shaderModule);
                gpipelineCI.stages.push_back(VK_SHADER_STAGE_TASK_BIT_EXT);
            }
        }
        else if (data.vertexEntryPoint.has_value())
        {
            gpipelineCI.useMeshShader = false;
            gpipelineCI.entryPoints.push_back(data.vertexEntryPoint.value());
            gpipelineCI.shaderModules.push_back(shaderModule);
            gpipelineCI.stages.push_back(VK_SHADER_STAGE_VERTEX_BIT);

            if (data.geometryEntryPoint.has_value())
            {
                gpipelineCI.entryPoints.push_back(data.geometryEntryPoint.value());
                gpipelineCI.shaderModules.push_back(shaderModule);
                gpipelineCI.stages.push_back(VK_SHADER_STAGE_GEOMETRY_BIT);
            }

            if (data.tesselletionControlEntryPoint.has_value())
            {
                gpipelineCI.entryPoints.push_back(data.tesselletionControlEntryPoint.value());
                gpipelineCI.shaderModules.push_back(shaderModule);
                gpipelineCI.stages.push_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
            }

            if (data.tesselletionEvaluationEntryPoint.has_value())
            {
                gpipelineCI.entryPoints.push_back(data.tesselletionEvaluationEntryPoint.value());
                gpipelineCI.shaderModules.push_back(shaderModule);
                gpipelineCI.stages.push_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
            }
        }

        gpipelineCI.colorAttachmentFormats.reserve(data.colorTargets.size());
        for (const auto& colorTarget : data.colorTargets)
        {
            gpipelineCI.colorAttachmentFormats.push_back(colorTarget.lock()->GetFormat());
        }

        // TODO: impl Depth Testing
        gpipelineCI.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

        pipeline->Create(*this, gpipelineCI);
        return pipeline;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateRenderTarget(const RenderTargetCreateInfo& createInfo) -> Result<RenderTarget>
{
    try
    {
        auto rt = std::make_shared<RenderTarget>();
        rt->Create(*this, createInfo);
        return rt;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateSequentialContext() -> Result<SequentialExecutionContext>
{
    try
    {
        SequentialExecutionContextCreateInfo createInfo = {};
        auto ctx = std::make_shared<SequentialExecutionContext>();
        ctx->Create(*this, createInfo);
        return ctx;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateShaderPack(const ShaderPackCreateInfo& createInfo) -> Result<ShaderPack>
{
    try
    {
        auto shaderPack = std::make_shared<ShaderPack>();
        shaderPack->Create(*this, createInfo);
        return shaderPack;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateTextureDictionary(const TextureCreateData& createData) -> Result<TextureDictionary>
{
    try
    {
        auto dict = std::make_shared<TextureDictionary>();
        // retrieve file names and info
        TextureDictionaryCreateInfo textureCI;
        std::vector<ktxTexture*> ktxTextures;

        for (const auto& [name, path] : createData.namePathPairs)
        {
            // get params
            ktxTexture* texture;
            auto res = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);

            textureCI.names.push_back(std::move(name));
            textureCI.formats.push_back(ktxTexture_GetVkFormat(texture));
            textureCI.widths.push_back(texture->baseWidth);
            textureCI.heights.push_back(texture->baseHeight);
            textureCI.layerCounts.push_back(texture->numLayers);
            textureCI.levelCounts.push_back(texture->numLevels);
            textureCI.isCubes.push_back(texture->isCubemap);

            ktxTextures.push_back(texture);
        }

        dict->Create(*this, textureCI);
        // TODO: transfer data

        // destroy ktx data
        for (const auto& tex : ktxTextures)
        {
            ktxTexture_Destroy(tex);
        }

        return dict;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateGPUMonotonicBufferResource(const Allocators::GPUMonotonicBufferResourceCreateInfo& createInfo) -> Result<Allocators::GPUMonotonicBufferResource>
{
    try
    {
        return std::make_shared<Allocators::GPUMonotonicBufferResource>(*this, createInfo);
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateLinearImageAllocator(const Allocators::LinearImageAllocCreateInfo& createInfo) -> Result<Allocators::LinearImageAlloc>
{
    try
    {
        auto alloc = std::make_shared<Allocators::LinearImageAlloc>();
        alloc->Create(*this, createInfo);
        return alloc;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}

auto Device::CreateLinearBufferSuballocator(const Allocators::LinearBufferSubAllocCreateInfo& createInfo) -> Result<Allocators::LinearBufferSubAlloc>
{
    try
    {
        auto alloc = std::make_shared<Allocators::LinearBufferSubAlloc>();
        alloc->Create(*this, createInfo);
        return alloc;
    }
    catch (LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch (...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }

}