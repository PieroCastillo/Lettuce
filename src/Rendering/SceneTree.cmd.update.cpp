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

namespace Lettuce::Rendering
{
    struct UpdateScenePartitionPayload
    {
        uint64_t dstPartitionAddr;
        uint64_t instancesAddr;
        uint32_t instanceCount;
    };
}

void SceneTreeCommandBuffer::Update(const UpdateScenePartitionDesc& update)
{
    auto deviceImpl = sTree->impl->device->GetImplementation();
    auto cmd = (VkCommandBuffer)cmdHandle;
    auto pUpdate = (VkPipeline)deviceImpl->pipelines.get(sTree->impl->pUpdateScenePartition).pipeline;
    auto& dt = deviceImpl->descriptorTables.get(sTree->impl->dtSceneTree);
    auto tCount = deviceImpl->props.preferredThreadCount;

    uint32_t bufferIdx = 0;
    uint64_t bufferOffset = 0;

    VkDescriptorBufferBindingInfoEXT bindingInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
        .address = dt.gpuAddress,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
    };

    auto payloadSize = sizeof(UpdateScenePartitionPayload);
    auto payload = (UpdateScenePartitionPayload*)alloca(payloadSize);

    payload->dstPartitionAddr = sTree->impl->scenePartitions.get(update.dstPartition).scenePartitionAddr;
    payload->instancesAddr = deviceImpl->memories.get(update.instanceUpdates).gpuAddress + update.instanceUpdatesByteOffset;
    payload->instanceCount = update.instanceUpdateCount;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pUpdate);
    vkCmdBindDescriptorBuffersEXT(cmd, 1, &bindingInfo);
    vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, dt.pipelineLayout, 0, 1, &bufferIdx, &bufferOffset);
    vkCmdPushConstants(cmd, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, payloadSize, &payload);
    vkCmdDispatch(cmd, ceil(update.instanceUpdateCount / tCount), 1, 1);
}