// standard headers
#include <algorithm>
#include <atomic>
#include <fstream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

// project headers
#include "Lettuce/Composition/api.hpp"
#include "Lettuce/Composition/CompositorImpl.hpp"
#include "Lettuce/Composition/HelperStructs.hpp"

using namespace Lettuce::Composition;

ShaderBinary CompositorImpl_LoadSpirv(Device* m_device, std::string_view path)
{
    auto shadersFile = std::ifstream(std::string(path), std::ios::ate | std::ios::binary);
    if (!shadersFile) throw std::runtime_error(std::string(path) + " does not exist");

    auto fileSize = (uint32_t)shadersFile.tellg();
    std::vector<uint32_t> shadersBuffer;
    shadersBuffer.resize(fileSize / sizeof(uint32_t));

    shadersFile.seekg(0);
    shadersFile.read((char*)shadersBuffer.data(), fileSize);

    ShaderBinaryDesc desc = {
        .bytecode = shadersBuffer,
    };
    return m_device->CreateShader(desc);
}

void CompositorImpl::Create(const CompositorDesc& desc)
{
    device = desc.device;
    swapchain = desc.swapchain;
    maxVisuals = desc.maxVisuals;
    maxMaterials = desc.maxMaterials;
    maxLights = desc.maxLights;
    maxAnimations = desc.maxAnimations;
    maxLinkedTextures = desc.maxLinkedTextures;

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Compute,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);

    DescriptorTableDesc dtDesc = {
        .sampledImageDescriptorCount= 1024,
        .samplerDescriptorCount = 8,
        .storageImageDescriptorCount = 1024,
    };
    descriptorTable = device->CreateDescriptorTable(dtDesc);

    CreateResources();
    CreatePipelines();
}

void CompositorImpl::Destroy()
{
    compositorThread.request_stop();
    // Destroy Pipelines
    device->Destroy(pTileBinningPass);
    device->Destroy(pTileRasterPass);
    device->Destroy(pPostprocessingPass);
    // Destroy Resources
    memAlloc.Destroy();

    device->Destroy(descriptorTable);
    device->Destroy(cmdAlloc);
}

void CompositorImpl::CreateResources()
{
    auto maxSize = (maxAnimations * sizeof(AnimationTokenGPUData)) +
        (maxVisuals * sizeof(VisualGPUData)) +
        (maxMaterials * sizeof(MaterialGPUData)) +
        (maxLights * sizeof(LightGPUData));

    Allocators::LinearAllocatorDesc linAllocDesc = {
        .maxBufferMemorySize = maxSize,
                .maxImageMemorySize = 16,
        .maxRenderTargetsMemorySize = 16,
    };
    memAlloc.Create(*device, linAllocDesc);
}

void CompositorImpl::CreatePipelines()
{
    auto compositionShader = CompositorImpl_LoadSpirv(device, "CompositionShader.spv");

    ComputePipelineDesc compDesc = {
        .compShaderBinary = compositionShader,
        .descriptorTable = descriptorTable,
    };

    compDesc.compEntryPoint = "compTilesBinningMain";
    pTileBinningPass = device->CreatePipeline(compDesc);

    compDesc.compEntryPoint = "compTileRasterPassMain";
    pTileRasterPass = device->CreatePipeline(compDesc);

    compDesc.compEntryPoint = "compTilePostprocessingPassMain";
    pPostprocessingPass = device->CreatePipeline(compDesc);

    device->Destroy(compositionShader);
}

void CompositorImpl::MainLoop()
{
    compositorThread = std::jthread([&](std::stop_token token) {
        while (!token.stop_requested())
        {
            BarrierDesc compCompBarrier[1] = { {
                .srcAccess = PipelineAccess::Write,
                .srcStage = PipelineStage::ComputeShader,
                .dstAccess = PipelineAccess::Write,
                .dstStage = PipelineStage::ComputeShader,
            }, };
            PushAllocationsDesc pushAllocs = {
                .descriptorTable = descriptorTable,
            };

            auto cmd = device->AllocateCommandBuffer(cmdAlloc);
            cmd.BindDescriptorTable(descriptorTable, PipelineBindPoint::Compute);

            cmd.BindPipeline(pTileBinningPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
            cmd.Barrier(std::span(compCompBarrier));

            cmd.BindPipeline(pTileRasterPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
            cmd.Barrier(std::span(compCompBarrier));

            cmd.BindPipeline(pPostprocessingPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
            cmd.Barrier(std::span(compCompBarrier));

            CommandBufferSubmitDesc cmdSubmit = {
                
            };
            device->Submit(cmdSubmit);
        }
        });
}