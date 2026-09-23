// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

// TODO: Implement CommandBuffer Queries
// TODO: Implement Queries for DrawIndirect

auto Device::CreateQueryHeap(const QueryHeapDesc& desc) -> QueryHeap
{
    DebugAssert(desc.maxQueryCount > 0, "maxQueryCount MUST be greater than 0");

    VkQueryPoolCreateInfo queryTimeCI = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = 2 * desc.maxQueryCount, // before/after query
    };

    VkQueryPoolCreateInfo queryCompCI = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        .queryCount = 1 * desc.maxQueryCount,
        .pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT,
    };

    VkQueryPoolCreateInfo queryPrimCI = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        .queryCount = 2 * desc.maxQueryCount,
        .pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
    };

    VkQueryPoolCreateInfo queryMeshCI = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        .queryCount = 3 * desc.maxQueryCount,
        .pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT | VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT,
    };

    VkQueryPool timeQuery;
    VkQueryPool compQuery;
    VkQueryPool primQuery;
    VkQueryPool meshQuery;

    handleResult(vkCreateQueryPool(impl->m_device, &queryTimeCI, nullptr, &timeQuery));
    handleResult(vkCreateQueryPool(impl->m_device, &queryCompCI, nullptr, &compQuery));
    handleResult(vkCreateQueryPool(impl->m_device, &queryPrimCI, nullptr, &primQuery));
    if (SupportMeshShader())
        handleResult(vkCreateQueryPool(impl->m_device, &queryMeshCI, nullptr, &meshQuery));

    return impl->queryHeaps.allocate({ timeQuery,compQuery,primQuery,meshQuery, desc.maxQueryCount });
}

void Device::Destroy(QueryHeap query)
{
    auto& info = impl->queryHeaps.get(query);
    vkDestroyQueryPool(impl->m_device, info.timeQuery, nullptr);
    vkDestroyQueryPool(impl->m_device, info.compQuery, nullptr);
    vkDestroyQueryPool(impl->m_device, info.primitiveQuery, nullptr);
    if (SupportMeshShader())
        vkDestroyQueryPool(impl->m_device, info.meshQuery, nullptr);

    impl->queryHeaps.release(query);
}
auto Device::GetResult(QueryHeap query, PipelineBindPoint bindPoint, uint32_t index) -> QueryResults
{
    QueryResults res = {};

    auto& queryData = impl->queryHeaps.get(query);

    uint64_t timeData[2];
    vkGetQueryPoolResults(impl->m_device, queryData.timeQuery, index * 2, 2, sizeof(timeData),
        timeData, sizeof(uint64_t), VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);

    auto delta = timeData[1] - timeData[0];
    res.ellapsedTime = static_cast<double>(delta) * static_cast<double>(impl->props.timestampPeriod);

    if (bindPoint == PipelineBindPoint::Compute)
    {
        uint64_t statsData[1];

        vkGetQueryPoolResults(impl->m_device, queryData.compQuery, index, 1,
            sizeof(statsData), statsData, sizeof(uint64_t), VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);

        res.computeShaderInvocations = statsData[0];
    }
    else if (bindPoint == PipelineBindPoint::Graphics)
    {
        uint64_t statsData[2];

        vkGetQueryPoolResults(impl->m_device, queryData.primitiveQuery, index, 1,
            sizeof(statsData), statsData, sizeof(uint64_t) * 2, VK_QUERY_RESULT_64_BIT);

        res.vertexShaderInvocations = statsData[0];
        res.fragmentShaderInvocations = statsData[1];

        if (SupportMeshShader())
        {
            uint64_t meshData[3];

            vkGetQueryPoolResults(impl->m_device, queryData.meshQuery, index, 1,
                sizeof(meshData), meshData, sizeof(uint64_t) * 3, VK_QUERY_RESULT_64_BIT);

            res.fragmentShaderInvocations = meshData[0];
            res.taskShaderInvocations = meshData[1];
            res.meshShaderInvocations = meshData[2];
        }
    }

    return res;
}