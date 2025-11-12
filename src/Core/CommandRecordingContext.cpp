// standard headers
#include <memory>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/CommandRecordingContext.hpp"

using namespace Lettuce::Core;

CommandRecordingContext::CommandRecordingContext()
{
    m_partialCommandList.reserve(32);
}

CommandsList CommandRecordingContext::GetCommands()
{
    return m_partialCommandList;
}

void CommandRecordingContext::BindPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    m_currentDraw.pipeline = pipeline->m_pipeline;
}

void CommandRecordingContext::BindDescriptorTable(const std::shared_ptr<DescriptorTable>& table)
{
    m_currentDraw.pipelineLayout = table->m_pipelineLayout;
    if (table->GetDescriptorSetLayoutCount() >= 0)
    {
        m_currentDraw.descriptorBufferAddress = table->GetAddress();
        // m_currentDraw.descriptorBufferOffsets
    }
}

void CommandRecordingContext::BindMesh(const MeshPool::Mesh& mesh)
{

}

void CommandRecordingContext::BeginRendering(uint32_t width, uint32_t height,
    const std::vector<std::reference_wrapper<const RenderTarget>>& colorTargets,
    const std::optional<std::reference_wrapper<const RenderTarget>> depthStencilTarget)
{
    renderingStartCommand command = {
        .width = width,
        .height = height,
    };
    m_width = width;
    m_height = height;

    if (depthStencilTarget.has_value())
    {
        auto& dsTarget = depthStencilTarget.value().get();
        auto layout = dsTarget.m_layout;
        auto view = dsTarget.m_imageView;

        VkRenderingAttachmentInfo ref = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = view,
            .imageLayout = layout,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };

        command.depthAttachment = ref;
        command.stencilAttachment = ref;
    }

    for (const auto& renderTarget : colorTargets)
    {
        auto& rt = renderTarget.get();

        VkRenderingAttachmentInfo ref = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = rt.m_imageView,
            .imageLayout = rt.m_layout,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        command.colorAttachments.push_back(ref);
    }

    m_partialCommandList.push_back(command);
    
    m_currentDraw = { m_width, m_height };
}

void CommandRecordingContext::EndRendering()
{
    m_partialCommandList.push_back(renderingEndCommand{});
}

void CommandRecordingContext::Draw(uint32_t vertexCount, uint32_t instanceCount)
{
    m_currentDraw.drawArgs = VkDrawIndirectCommand{ vertexCount, instanceCount, 0, 0 };
    m_partialCommandList.push_back(m_currentDraw);
    m_currentDraw = { m_width, m_height };
}