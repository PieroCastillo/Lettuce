// standard headers
#include <array>
#include <memory_resource>
#include <string>
#include <stdexcept>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Foundations/Loader.hpp"
#include "Lettuce/Rendering/Shapes/ShapeRenderPass.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;
using namespace Lettuce::Rendering::Shapes;

ShapeRenderPass::ShapeRenderPass(const ShapeRenderPassDesc& desc)
{
    Create(desc);
}

ShapeRenderPass::~ShapeRenderPass()
{
    Destroy();
}

ShapeRenderPass::ShapeRenderPass(ShapeRenderPass&& other) noexcept
    : BasePass(std::exchange(other.m_device, nullptr), std::move(other.m_descriptorTable)),
    pPass(std::move(other.pPass)),
    pBuild(std::move(other.pBuild)),
    m_indirect(std::move(other.m_indirect)),
    m_indirectCommands(std::move(other.m_indirectCommands))
{
}

ShapeRenderPass& ShapeRenderPass::operator=(ShapeRenderPass&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_device = std::exchange(other.m_device, nullptr);
        m_descriptorTable = std::move(other.m_descriptorTable);

        pPass = std::move(other.pPass);
        pBuild = std::move(other.pBuild);
        m_indirect = std::move(other.m_indirect);
        m_indirectCommands = std::move(other.m_indirectCommands);
    }

    return *this;
}

void ShapeRenderPass::Create(const ShapeRenderPassDesc& desc)
{
    if (m_device)
        throw std::logic_error("ShapeRenderPass::Create cannot be called from initizalized Device.");

    m_device = &desc.device;
    m_descriptorTable = desc.descriptorTable;

    try
    {
        // load spv
        auto path = std::string("./shaders/Shapes/ShapeRenderPass.spv");
        auto shader = Loader::LoadSpirv(m_device, path);

        auto formats = std::array{ Format::Raw_RGBA32_SFloat, Format::Atomic_R32_UInt };

        ComputePipelineDesc compDesc = {
            .compEntryPoint = "generateDrawCalls",
            .compShaderBinary = shader,
            .descriptorTable = desc.descriptorTable,
        };
        pBuild = m_device->CreatePipeline(compDesc);

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

        m_device->Destroy(shader);

        m_indirect = m_device->CreateIndirectSet({ IndirectType::Draw, desc.maxCommands, 0 });
        m_indirectCommands = m_device->GetIndirectSetView(m_indirect);
    }
    catch (...)
    {
        throw;
    }
}

void ShapeRenderPass::Destroy()
{
    if (!m_device)
        return;

    m_device->Destroy(m_indirect);
    m_device->Destroy(pPass);
    m_device->Destroy(pBuild);

    m_device = nullptr;
    m_descriptorTable = {};
    pBuild = {};
    pPass = {};
    m_indirect = {};
    m_indirectCommands = {};
}

void ShapeRenderPass::Record(CommandBuffer& cmd, const ShapeRenderPassRecordDesc& desc)
{
    BarrierDesc bCompInd = {
        .srcAccess = PipelineAccess::Write,
        .srcStage = PipelineStage::ComputeShader,
        .dstAccess = PipelineAccess::Read,
        .dstStage = PipelineStage::DrawIndirect,
    };

    AttachmentDesc colorAttachments[2] = {
    {
        .renderTarget = desc.rtColorOutput,
        .loadOp = desc.rtLoadOp,
    },
    {
        .renderTarget = desc.rtPick,
        .loadOp = desc.rtLoadOp,
    }
    };
    AttachmentDesc depthAttachment = {
        .renderTarget = desc.rtDepth,
        .loadOp = desc.rtLoadOp,
    };

    RenderPassDesc renderPassDesc = {
        .width = desc.fbWidth,
        .height = desc.fbHeight,
        .colorAttachments = std::span(colorAttachments),
        .depthStencilAttachment = depthAttachment,
        .presentAttachmentIdx = 0,
    };

    std::array<std::byte, sizeof(PushAllocationBinding) * 9> buffer;
    std::pmr::monotonic_buffer_resource pool{ buffer.data(), buffer.size() };
    auto allocs = std::pmr::vector<PushAllocationBinding>{ {
        desc.sceneViewData.getView(),
        desc.materialShadingData.getView(),
        desc.instanceData.getView(),
        desc.materials.getView(),
        desc.instances.getView(),
        desc.spheres.getView(),
        desc.prisms.getView(),
        desc.cylinders.getView(),
        m_indirectCommands,
    }, &pool };

    constexpr uint32_t computeNumThreadsX = 32;
    uint32_t groupCountX = (desc.instances.size() + (computeNumThreadsX - 1)) / computeNumThreadsX;

    cmd.BindPipeline(pBuild);
    cmd.BindDescriptorTable(m_descriptorTable, PipelineBindPoint::Compute);
    cmd.PushAllocations({ allocs, m_descriptorTable });
    cmd.Dispatch(groupCountX, 1, 1);

    cmd.Barrier({ bCompInd });

    cmd.BeginRendering(renderPassDesc);
    cmd.BindPipeline(pPass);
    cmd.BindDescriptorTable(m_descriptorTable, PipelineBindPoint::Graphics);
    cmd.PushAllocations({ allocs, m_descriptorTable });
    // TODO: apply culling
    cmd.ExecuteIndirect({ m_indirect, 0, desc.instances.size() });
    cmd.EndRendering();
}