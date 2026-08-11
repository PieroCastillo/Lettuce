// standard headers
#include <memory>
#include <memory_resource>
#include <fstream>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/Debug/DebugPass.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;
using namespace Lettuce::Rendering::Debug;

DebugPass::DebugPass(const DebugPassDesc& desc)
{
    Create(desc);
}

DebugPass::~DebugPass()
{
    Destroy();
}

DebugPass::DebugPass(DebugPass&& other) noexcept
    : m_device(std::exchange(other.m_device, nullptr)),
    dtPass(std::move(other.dtPass)),
    pPass(std::move(other.pPass)),
    pBuildCommands(std::move(other.pBuildCommands)),
    isPass(std::move(other.isPass)),
    mvIndirectDrawCommands(std::move(other.mvIndirectDrawCommands))
{
}

DebugPass& DebugPass::operator=(DebugPass&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_device = std::exchange(other.m_device, nullptr);

        dtPass = std::move(other.dtPass);
        pPass = std::move(other.pPass);
        pBuildCommands = std::move(other.pBuildCommands);
        isPass = std::move(other.isPass);
        mvIndirectDrawCommands = std::move(other.mvIndirectDrawCommands);
    }

    return *this;
}

void DebugPass::Create(const DebugPassDesc& desc)
{
    if (m_device)
        throw std::logic_error("SceneView::Create cannot be called from initizalized Device.");

    m_device = &desc.device;
    dtPass = desc.descriptorTable;

    try
    {
        // load spv
        std::vector<uint32_t> shaderBuffer;
        auto path = std::string("./src/Rendering/Debug/DebugPass.spv");
        auto shaderFile = std::ifstream(path, std::ios::ate | std::ios::binary);
        if (!shaderFile) throw std::runtime_error(path + " does not exist");
        auto fileSize = (uint32_t)shaderFile.tellg();
        shaderBuffer.resize(fileSize / sizeof(uint32_t));
        shaderFile.seekg(0);
        shaderFile.read((char*)shaderBuffer.data(), fileSize);
        auto shader = m_device->CreateShader({ shaderBuffer });

        auto formats = std::array{ desc.colorOutputFormat, Format::Atomic_R32_UInt };

        ComputePipelineDesc compDesc = {
            .compEntryPoint = "buildCommandsMain",
            .compShaderBinary = shader,
            .descriptorTable = desc.descriptorTable,
        };
        pBuildCommands = m_device->CreatePipeline(compDesc);

        PrimitiveShadingPipelineDesc pipelineDesc = {
            .fragmentShadingRate = false,
            .vertEntryPoint = "vertMain",
            .fragEntryPoint = "fragMain",
            .vertShaderBinary = shader,
            .fragShaderBinary = shader,
            .colorAttachmentFormats = formats,
            .depthStencilAttachmentFormat = Format::Universal_Depth_D32_SFloat,
            .descriptorTable = desc.descriptorTable,
        };
        pPass = m_device->CreatePipeline(pipelineDesc);
        isPass = m_device->CreateIndirectSet({ IndirectType::Draw, desc.maxCulledInstances, 0 });
        mvIndirectDrawCommands = m_device->GetIndirectSetView(isPass);

        m_device->Destroy(shader);
    }
    catch (...)
    {
        throw;
    }
}

void DebugPass::Destroy()
{
    if (!m_device)
        return;

    m_device->Destroy(isPass);
    m_device->Destroy(pPass);
    m_device->Destroy(pBuildCommands);

    m_device = nullptr;
    dtPass = {};
    pBuildCommands = {};
    pPass = {};
    isPass = {};
    mvIndirectDrawCommands = {};
}

void DebugPass::Record(CommandBuffer& cmd, const DebugPassRecordDesc& desc)
{
    BarrierDesc bCompInd[] = { {
        .srcAccess = PipelineAccess::Write,
        .srcStage = PipelineStage::ComputeShader,
        .dstAccess = PipelineAccess::Read,
        .dstStage = PipelineStage::DrawIndirect,
    }, };

    AttachmentDesc colorAttachments[2] = {
    {
        .renderTarget = desc.rtColorOutput,
        .loadOp = LoadOp::Clear,
    },
    {
        .renderTarget = desc.rtPick,
        .loadOp = LoadOp::Clear,
    }
    };
    AttachmentDesc depthAttachment = {
        .renderTarget = desc.rtDepth,
        .loadOp = LoadOp::Clear,
    };

    RenderPassDesc renderPassDesc = {
        .width = desc.fbWidth,
        .height = desc.fbHeight,
        .colorAttachments = std::span(colorAttachments),
        .depthStencilAttachment = depthAttachment,
        .presentAttachmentIdx = 0,
    };

    auto allocs = std::array<PushAllocationBinding, 7> {
        desc.sceneViewData.getView(),
            mvIndirectDrawCommands,
            desc.positions.getView(),
            desc.indices.getView(),
            desc.clusters.getView(),
            desc.meshes.getView(),
            desc.culledInstances.getView(),
    };

    PushAllocationsDesc pushAllocs = {
        .allocations = allocs,
        .descriptorTable = dtPass,
    };

    constexpr uint32_t computeNumThreadsX = 32;
    uint32_t groupCountX = (desc.culledInstances.size() + (computeNumThreadsX - 1)) / computeNumThreadsX;

    cmd.BindPipeline(pBuildCommands);
    cmd.BindDescriptorTable(dtPass, PipelineBindPoint::Compute);
    cmd.PushAllocations(pushAllocs);
    cmd.Dispatch(groupCountX, 1, 1);

    cmd.Barrier(bCompInd);

    cmd.BeginRendering(renderPassDesc);

    cmd.BindPipeline(pPass);
    cmd.BindDescriptorTable(dtPass, PipelineBindPoint::Graphics);
    cmd.PushAllocations(pushAllocs);
    cmd.ExecuteIndirect({ isPass, 0, desc.culledInstances.size() });

    cmd.EndRendering();
}