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

/*
TODO:
we need to call a dispatch kernel to configure indirect calls
fillKernel(Updates) -> dispatch indirect commands
after that we can call Dispatch indirect and do the updates
*/
void SceneTreeCommandBuffer::Update(const UpdateScenePartitionDesc& update)
{
    auto deviceImpl = sTree->impl->device->GetImplementation();
    auto cmd = (VkCommandBuffer)cmdHandle;
    auto pipeline = (VkPipeline)deviceImpl->pipelines.get(sTree->impl->pUpdateScenePartition).pipeline;
    auto& dt = deviceImpl->descriptorTables.get(sTree->impl->dtSceneTree);
    auto tCount = deviceImpl->props.preferredThreadCount;

    uint32_t bufferIdx = 0;
    uint64_t bufferOffset = 0;

    VkDescriptorBufferBindingInfoEXT bindingInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
        .address = dt.gpuAddress,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
    };

    auto payloadSize = 128;
    auto payload = (uint8_t*)alloca(payloadSize);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorBuffersEXT(cmd, 1, &bindingInfo);
    vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, dt.pipelineLayout, 0, 1, &bufferIdx, &bufferOffset);
    vkCmdPushConstants(cmd, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, payloadSize, &payload);
    // vkCmdDispatchIndirect(cmd, , );
}