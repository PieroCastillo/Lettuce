// standard headers
#include <memory>
#include <memory_resource>
#include <fstream>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/DeferredPlus/MaterialShadingPass.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;
using namespace Lettuce::Rendering::DeferredPlus;

// TODO: use a base class for passes?
MaterialShadingPass::MaterialShadingPass(const GeometryPassDesc& desc)
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

void MaterialShadingPass::Create(const GeometryPassDesc& desc)
{
   if (m_device)
        throw std::logic_error("MaterialShadingPass::Create cannot be called from initizalized Device.");

    m_device = &desc.device;
    dtPass = desc.descriptorTable;

    try
    {
        // load spv
        std::vector<uint32_t> shaderBuffer;
        auto path = std::string("./shaders/DeferredPlus/MaterialShadingPass.spv");
        auto shaderFile = std::ifstream(path, std::ios::ate | std::ios::binary);
        if (!shaderFile) throw std::runtime_error(path + " does not exist");
        auto fileSize = (uint32_t)shaderFile.tellg();
        shaderBuffer.resize(fileSize / sizeof(uint32_t));
        shaderFile.seekg(0);
        shaderFile.read((char*)shaderBuffer.data(), fileSize);
        auto shader = m_device->CreateShader({ shaderBuffer });

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
        isPass = m_device->CreateIndirectSet({ IndirectType::Draw, desc.maxCulledInstances, 0 });
        mvIndirectDrawCommands = m_device->GetIndirectSetView(isPass);

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

void MaterialShadingPass::Record(CommandBuffer& cmd, const GeometryPassRecordDesc& desc)
{

}