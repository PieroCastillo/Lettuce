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
    device = &(desc.device);

    CommandAllocatorDesc cmdAllocDesc = {
        .queueType = QueueType::Compute,
    };
    cmdAlloc = device->CreateCommandAllocator(cmdAllocDesc);

    DescriptorTableDesc dtDesc = {

    };
    descriptorTable = device->CreateDescriptorTable(dtDesc);

    CreateResources();
    CreatePipelines();
}

void CompositorImpl::Destroy()
{
    // Destroy Pipelines
    device->Destroy(pEffectPass);
    device->Destroy(pLightingPass);
    device->Destroy(pBrushesPass);
    device->Destroy(pObjectPass);
    device->Destroy(pAnimationEvaluationPass);
    // Destroy Resources

    device->Destroy(descriptorTable);
    device->Destroy(cmdAlloc);
}

void CompositorImpl::CreateResources()
{

}

void CompositorImpl::CreatePipelines()
{
    auto compositionShader = CompositorImpl_LoadSpirv(device, "CompositionShader.spv");

    ComputePipelineDesc compDesc = {
        .compShaderBinary = compositionShader,
        .descriptorTable = descriptorTable,
    };

    compDesc.compEntryPoint = "animationsPassMain";
    pAnimationEvaluationPass = device->CreatePipeline(compDesc);

    compDesc.compEntryPoint = "visualsPassMain";
    pObjectPass = device->CreatePipeline(compDesc);

    compDesc.compEntryPoint = "brushesPassMain";
    pBrushesPass = device->CreatePipeline(compDesc);

    compDesc.compEntryPoint = "lightsPassMain";
    pLightingPass = device->CreatePipeline(compDesc);

    compDesc.compEntryPoint = "effectsPassMain";
    pEffectPass = device->CreatePipeline(compDesc);

    device->Destroy(compositionShader);
}

void CompositorImpl::MainLoop()
{
    compositorThread = std::thread([&]() {
        while (!stop.load(std::memory_order_acquire))
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

            // TODO: impl correct Dispatch() values
            cmd.BindPipeline(pAnimationEvaluationPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
            cmd.Barrier(std::span(compCompBarrier));

            cmd.BindPipeline(pObjectPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
            cmd.Barrier(std::span(compCompBarrier));

            cmd.BindPipeline(pBrushesPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
            cmd.Barrier(std::span(compCompBarrier));

            cmd.BindPipeline(pLightingPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
            cmd.Barrier(std::span(compCompBarrier));

            cmd.BindPipeline(pEffectPass);
            cmd.PushAllocations(pushAllocs);
            cmd.Dispatch(1, 1, 1);
        }
        });
}