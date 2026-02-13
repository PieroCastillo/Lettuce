// standard headers
#include <memory>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/api.hpp"
#include "Lettuce/Rendering/HelperStructs.hpp"
#include "Lettuce/Rendering/SceneGraphImpl.hpp"

using namespace Lettuce::Rendering;
using namespace Lettuce::Core;

void SceneGraph::Create(const SceneGraphDesc& desc)
{
    impl = new SceneGraphImpl;
}

void SceneGraph::Destroy()
{
    delete impl;
}

void SceneGraph::Record(CommandBuffer& cmd)
{

    // auto& nodeInfo = impl->nodes.get(node);

    // auto& mat = nodeInfo.material;
    // auto allocs = std::array{
    //     std::pair(0u, impl->sceneMemoryView),
    //     std::pair(1u, impl->lightMemoryView),
    //     std::pair(2u, nodeInfo.materialDataView),
    //     std::pair(3u, nodeInfo.geometryDataView),
    //     std::pair(4u, nodeInfo.vertexBuffer),
    //     std::pair(5u, nodeInfo.indexBuffer), };


    // PushAllocationsDesc pushDesc = {
    //     .allocations = std::span(allocs),
    //     .descriptorTable = mat->descriptorTable,
    // };
    // ExecuteIndirectDesc execIndirectDesc = {
    //     .indirectSet = nodeInfo.indirectSet,
    //     .maxDrawCount = 4096,
    // };

    // uint32_t instanceCount = 0;
    // // // object-cull pass
    // // cmd.BindDescriptorTable(cullpass->descriptorTable, PipelineBindPoint::Compute);
    // // cmd.BindPipeline(cullpass->passPipeline);
    // // cmd.PushAllocations(pushDesc);
    // // cmd.Dispatch((instanceCount/32) + 1,1,1);

    // // vi-buffer pass
    // cmd.BindDescriptorTable(viPass->descriptorTable, PipelineBindPoint::Graphics);
    // cmd.BindPipeline(viPass->passPipeline);
    // cmd.PushAllocations(pushDesc);
    // cmd.ExecuteIndirect(execIndirectDesc);

    // // material shading pass
    // for(auto& mat : materials)
    // {
    //     cmd.BindDescriptorTable(mat->descriptorTable, PipelineBindPoint::Compute);
    //     cmd.BindPipeline(mat->materialPipeline);
    //     cmd.Dispatch((instanceCount/32) + 1,1,1);
    // }
}