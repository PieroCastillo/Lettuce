/*
Created by @PieroCastillo on 2025-07-28
*/
#ifndef LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP
#define LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP

// standard headers
#include <string>
#include <vector>
#include <variant>

// project headers
#include "common.hpp"
#include "Commands.hpp"
#include "DescriptorTable.hpp"
#include "Mesh.hpp"
#include "Pipeline.hpp"
#include "RenderingInfo.hpp"
#include "RenderTarget.hpp"

namespace Lettuce::Core
{
    /// @brief Stores commands for lazy recording
    class CommandRecordingContext
    {
    private:
        CommandsList m_partialCommandList;
        drawCommand m_currentDraw;
        computeCommand m_currentCompute;
        uint32_t m_width, m_height;
        std::shared_ptr<DescriptorTable> m_currentTable;
    public:
        CommandRecordingContext();

        CommandsList GetCommands();

        // Graphics & Compute commands
        void BindIndexStream(const BufferHandle& buffer);
        void BindVertexStreams(const std::vector<BufferHandle>& buffers);
        void BindPipeline(const std::shared_ptr<Pipeline>& pipeline);
        void BindDescriptorTable(const std::shared_ptr<DescriptorTable>& table);
        void BindDescriptorSet(const std::string& setName);
        void BindMesh(const MeshPool::Mesh& mesh);

        void BeginRendering(uint32_t width, uint32_t height,
                            const std::vector<std::reference_wrapper<const RenderTarget>>& colorTargets,
                            const std::optional<std::reference_wrapper<const RenderTarget>> depthStencilTarget);
        void EndRendering();
        // Graphics commands
        // void ClearColorImage();
        // void ClearDepthStencilImage();
        // void ClearAttachments();

        void Draw(uint32_t vertexCount, uint32_t instanceCount);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount);
        // void DrawMeshTasks();

        // void DrawIndexed(const DrawIndexedArgs& args);
        // void DrawIndirect(const DrawIndirectArgs& args);
        // void Dispatch(const DispatchArgs& args);
        void Flush();
    };
}
#endif // LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP