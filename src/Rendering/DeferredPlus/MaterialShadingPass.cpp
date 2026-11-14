// standard headers
#include <array>
#include <memory_resource>
#include <string>
#include <stdexcept>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Foundations/Loader.hpp"
#include "Lettuce/Rendering/DeferredPlus/MaterialShadingPass.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;
using namespace Lettuce::Rendering::DeferredPlus;

MaterialShadingPass::MaterialShadingPass(const MaterialShadingPassDesc& desc)
{
    Create(desc);
}

MaterialShadingPass::~MaterialShadingPass()
{
    Destroy();
}

MaterialShadingPass::MaterialShadingPass(MaterialShadingPass&& other) noexcept
    : BasePass(std::exchange(other.m_device, nullptr), std::move(other.m_descriptorTable)),
    pPass(std::move(other.pPass))
{
}

MaterialShadingPass& MaterialShadingPass::operator=(MaterialShadingPass&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_device = std::exchange(other.m_device, nullptr);
        m_descriptorTable = std::move(other.m_descriptorTable);

        pPass = std::move(other.pPass);
    }

    return *this;
}

void MaterialShadingPass::Create(const MaterialShadingPassDesc& desc)
{
    if (m_device)
        throw std::logic_error("MaterialShadingPass::Create cannot be called from initizalized Device.");

    m_device = &desc.device;
    m_descriptorTable = desc.descriptorTable;

    try
    {
        // load spv
        auto path = std::string("./shaders/DeferredPlus/MaterialShadingPass.spv");
        auto shader = Loader::LoadSpirv(m_device, path);

        auto formats = std::array{ Format::Raw_RGBA32_SFloat, Format::Atomic_R32_UInt };

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
    }
    catch (...)
    {
        throw;
    }
}

void MaterialShadingPass::Destroy()
{
    if (!m_device)
        return;

    m_device->Destroy(pPass);

    m_device = nullptr;
    m_descriptorTable = {};
    pPass = {};
}

void MaterialShadingPass::Record(CommandBuffer& cmd, const MaterialShadingPassRecordDesc& desc)
{
    AttachmentDesc colorAttachment = {
        .renderTarget = desc.rtColorOutput,
        .loadOp = LoadOp::Load,
    };
    AttachmentDesc depthAttachment = {
        .renderTarget = desc.rtDepth,
        .loadOp = LoadOp::Clear,
    };

    RenderPassDesc renderPassDesc = {
        .width = desc.fbWidth,
        .height = desc.fbHeight,
        .colorAttachments = std::span(&colorAttachment, 1),
        .depthStencilAttachment = depthAttachment,
        .presentAttachmentIdx = 0,
    };

    std::array<std::byte, sizeof(PushAllocationBinding) * 6> buffer;
    std::pmr::monotonic_buffer_resource pool{ buffer.data(), buffer.size() };
    auto allocs = std::pmr::vector<PushAllocationBinding>{ {
        desc.sceneViewData.getView(),
        desc.materialShadingData.getView(),
        desc.materialStorage.getView(),
        desc.sunLightData.getView(),
        desc.spotLightData.getView(),
        desc.cascadeShadowMapData.getView(),
    }, &pool };

    cmd.BeginRendering(renderPassDesc);
    cmd.BindPipeline(pPass);
    cmd.BindDescriptorTable(m_descriptorTable, PipelineBindPoint::Graphics);
    cmd.PushAllocations({ allocs, m_descriptorTable });
    cmd.Draw(6, 1);
    cmd.EndRendering();
}