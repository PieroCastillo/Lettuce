// standard headers
#include <memory>
#include <memory_resource>
#include <fstream>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Foundations/Loader.hpp"
#include "Lettuce/Rendering/DeferredPlus/MaterialShadingPass.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;
using namespace Lettuce::Rendering::DeferredPlus;

// TODO: use a base class for passes?
MaterialShadingPass::MaterialShadingPass(const MaterialShadingPassDesc& desc)
{
    Create(desc);
}

MaterialShadingPass::~MaterialShadingPass()
{
    Destroy();
}

MaterialShadingPass::MaterialShadingPass(MaterialShadingPass&& other) noexcept
    : m_device(std::exchange(other.m_device, nullptr)),
    dtPass(std::move(other.dtPass)),
    pPass(std::move(other.pPass)),
    isPass(std::move(other.isPass)),
    mvIndirectDrawCommands(std::move(other.mvIndirectDrawCommands))
{
}

MaterialShadingPass& MaterialShadingPass::operator=(MaterialShadingPass&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_device = std::exchange(other.m_device, nullptr);

        dtPass = std::move(other.dtPass);
        pPass = std::move(other.pPass);
        isPass = std::move(other.isPass);
        mvIndirectDrawCommands = std::move(other.mvIndirectDrawCommands);
    }

    return *this;
}

void MaterialShadingPass::Create(const MaterialShadingPassDesc& desc)
{
   if (m_device)
        throw std::logic_error("MaterialShadingPass::Create cannot be called from initizalized Device.");

    m_device = &desc.device;
    dtPass = desc.descriptorTable;

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

    m_device->Destroy(isPass);
    m_device->Destroy(pPass);

    m_device = nullptr;
    dtPass = {};
    pPass = {};
    isPass = {};
    mvIndirectDrawCommands = {};
}

void MaterialShadingPass::Record(CommandBuffer& cmd, const MaterialShadingPassRecordDesc& desc)
{

}