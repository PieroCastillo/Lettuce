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
build  cblas: dispatch( ceil(clasCount/threadPerGroup) ) per build
build  ptlas: dispatch( ceil(instanceCount/threadPerGroup) ) per build
update ptlas: dispatch( ceil(instanceCount/threadPerGroup) ) per update
*/

namespace Lettuce::Rendering
{
    struct BuildGeometryClusterPayload
    {
        uint64_t dstGeometryClusterAddr;
        uint64_t clustersAddr;
        uint64_t positionsAddr;
        uint64_t indicesAddr;

        uint32_t clusterCount;
        uint32_t positionStride;
        uint32_t flags;
    };

    struct BuildScenePartitionPayload
    {
        uint64_t dstScenePartitionAddr;
        uint64_t instancesAddr;
        uint32_t instanceCount;
    };
}

void SceneTreeCommandBuffer::Build(const BuildGeometryClusterDesc& build)
{
    auto deviceImpl = sTree->impl->device->GetImplementation();
    auto& mvs = deviceImpl->memories;
    auto cmd = (VkCommandBuffer)cmdHandle;
    auto pBuildGc = (VkPipeline)deviceImpl->pipelines.get(sTree->impl->pBuildGeometryCluster).pipeline;
    auto& dt = deviceImpl->descriptorTables.get(sTree->impl->dtSceneTree);
    auto tCount = deviceImpl->props.preferredThreadCount;

    uint32_t bufferIdx = 0;
    uint64_t bufferOffset = 0;

    VkDescriptorBufferBindingInfoEXT bindingInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
        .address = dt.gpuAddress,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
    };

    auto payloadSize = sizeof(BuildGeometryClusterPayload);
    auto payload = (BuildGeometryClusterPayload*)alloca(payloadSize);

    payload->dstGeometryClusterAddr = sTree->impl->clusterDatas.get(build.dstCluster).geometryClusterAddr;
    payload->positionsAddr = mvs.get(build.positions).gpuAddress + build.positionsByteOffset;
    payload->indicesAddr = mvs.get(build.indices).gpuAddress + build.indicesByteOffset;
    payload->clustersAddr = mvs.get(build.clusterDescs).gpuAddress + build.clusterDescsByteOffset;

    payload->clusterCount = build.clusterCount;
    payload->positionStride = build.vertexStride;
    payload->flags = (uint32_t)build.flags;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pBuildGc);
    vkCmdBindDescriptorBuffersEXT(cmd, 1, &bindingInfo);
    vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, dt.pipelineLayout, 0, 1, &bufferIdx, &bufferOffset);
    vkCmdPushConstants(cmd, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, payloadSize, &payload);
    vkCmdDispatch(cmd, ceil(build.clusterCount / tCount), 1, 1);
}

void SceneTreeCommandBuffer::Build(const BuildScenePartitionDesc& build)
{
    auto deviceImpl = sTree->impl->device->GetImplementation();
    auto& mvs = deviceImpl->memories;
    auto cmd = (VkCommandBuffer)cmdHandle;
    auto pBuildSp = (VkPipeline)deviceImpl->pipelines.get(sTree->impl->pBuildScenePartition).pipeline;
    auto& dt = deviceImpl->descriptorTables.get(sTree->impl->dtSceneTree);
    auto tCount = deviceImpl->props.preferredThreadCount;

    uint32_t bufferIdx = 0;
    uint64_t bufferOffset = 0;

    VkDescriptorBufferBindingInfoEXT bindingInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
        .address = dt.gpuAddress,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
    };

    auto payloadSize = sizeof(BuildScenePartitionPayload);
    auto payload = (BuildScenePartitionPayload*)alloca(payloadSize);

    payload->dstScenePartitionAddr = sTree->impl->scenePartitions.get(build.dstPartition).scenePartitionAddr;
    payload->instancesAddr = mvs.get(build.instanceWrites).gpuAddress + build.instanceWritesByteOffset;
    payload->instanceCount = build.instanceCount;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pBuildSp);
    vkCmdBindDescriptorBuffersEXT(cmd, 1, &bindingInfo);
    vkCmdSetDescriptorBufferOffsetsEXT(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, dt.pipelineLayout, 0, 1, &bufferIdx, &bufferOffset);
    vkCmdPushConstants(cmd, dt.pipelineLayout, VK_SHADER_STAGE_ALL, 0, payloadSize, &payload);
    vkCmdDispatch(cmd, ceil(build.instanceCount/ tCount), 1, 1);
}