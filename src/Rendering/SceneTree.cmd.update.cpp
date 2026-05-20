// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/common.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Rendering/SceneTree.hpp"
#include "Lettuce/Rendering/SceneTreeImpl.hpp"

using namespace Lettuce::Rendering;
using namespace Lettuce::Core;

void SceneTreeCommandBuffer::Update(const UpdateGeometryClusterInstanceDesc& update)
{
    auto deviceImpl = sTree->impl->device->GetImplementation();
    auto cmd = (VkCommandBuffer)cmdHandle;
    auto pipeline = (VkPipeline)deviceImpl->pipelines.get(sTree->impl->pBuildGeometryClusterInstance).pipeline;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    
    vkCmdDispatch(cmd, 32, 1, 1);
}

void SceneTreeCommandBuffer::Update(const UpdateScenePartitionDesc& update)
{

}