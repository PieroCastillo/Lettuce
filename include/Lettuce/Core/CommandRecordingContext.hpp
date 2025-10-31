/*
Created by @PieroCastillo on 2025-07-28
*/
#ifndef LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP
#define LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP

// standard headers
#include <vector>
#include <variant>

// project headers
#include "common.hpp"
#include "DescriptorTable.hpp"
#include "Mesh.hpp"
#include "Pipeline.hpp"
#include "RenderingInfo.hpp"
#include "RenderTarget.hpp"

namespace Lettuce::Core
{
    struct drawCommand
    {
        
    };

    struct computeCommand
    {

    };

    using CommandsList = std::vector<std::variant<drawCommand, computeCommand>>;

    /// @brief Stores commands for lazy recording
    class CommandRecordingContext
    {
    public:
        std::vector<drawCommand> drawCommands; 
        std::vector<computeCommand> computeCommands;

        void record(VkCommandBuffer cmd);

        // Graphics & Compute commands
        //void BindIndexBuffer(const std::shared_ptr<Buffer>& buffer);
        //void BindVertexBuffers(const std::vector<std::shared_ptr<Buffer>>& buffers);
        void BindPipeline(const std::shared_ptr<Pipeline>& pipeline);
        void BindDescriptorTable(const std::shared_ptr<DescriptorTable>& table);
        void BindMesh(const MeshPool::Mesh& mesh);

        void SetOutputs(const std::vector<std::weak_ptr<RenderTarget>>& targets);
        // Graphics commands
        // void ClearColorImage();
        // void ClearDepthStencilImage();
        // void ClearAttachments();

        // void Draw();
        // void DrawMeshTasks();

        // void DrawIndexed(const DrawIndexedArgs& args);
        // void DrawIndirect(const DrawIndirectArgs& args);
        // void Dispatch(const DispatchArgs& args);
    };
}
#endif // LETTUCE_CORE_COMMAND_RECORDING_CONTEXT_HPP