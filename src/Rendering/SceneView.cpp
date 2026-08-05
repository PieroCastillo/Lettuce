// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/SceneView.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;

SceneView::SceneView(const SceneViewDesc& desc)
{
    Create(desc);
}

SceneView::~SceneView()
{
    Destroy();
}

SceneView::SceneView(SceneView&& other) noexcept
    : m_device(std::exchange(other.m_device, nullptr)),
    m_cmdAlloc(std::move(other.m_cmdAlloc)),
    positionTable(std::move(other.positionTable)),
    normalTable(std::move(other.normalTable)),
    tangentTable(std::move(other.tangentTable)),
    texCoord0Table(std::move(other.texCoord0Table)),
    indexTable(std::move(other.indexTable)),
    clusterTable(std::move(other.clusterTable)),
    meshTable(std::move(other.meshTable)),
    instanceTable(std::move(other.instanceTable))
{
}

SceneView& SceneView::operator=(SceneView&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_device = std::exchange(other.m_device, nullptr);
        m_cmdAlloc = std::move(other.m_cmdAlloc);

        positionTable = std::move(other.positionTable);
        normalTable = std::move(other.normalTable);
        tangentTable = std::move(other.tangentTable);
        texCoord0Table = std::move(other.texCoord0Table);

        indexTable = std::move(other.indexTable);
        clusterTable = std::move(other.clusterTable);
        meshTable = std::move(other.meshTable);
        instanceTable = std::move(other.instanceTable);
    }

    return *this;
}

void SceneView::Create(const SceneViewDesc& desc)
{
    if (m_device)
        throw std::logic_error("SceneView::Create cannot be called from initizalized Device.");

    m_device = &desc.device;
    m_cmdAlloc = m_device->CreateCommandAllocator({ QueueType::Copy });

    // fusion buffers
    std::vector<SceneViewData> scenes;
    std::vector<float3> positions;
    std::vector<float3> normals;
    std::vector<float4> tangents;
    std::vector<float2> texCoords0;
    std::vector<uint8_t> indices;
    std::vector<ClusterStorage> clusters;
    std::vector<MeshStorage>  meshes;
    std::vector<InstanceStorage> instances;

    try
    {
        for (const GeometrySource& src : desc.sources)
        {
            const auto positionBase = (uint32_t)positions.size();
            const auto normalBase = (uint32_t)normals.size();
            const auto tangentBase = (uint32_t)tangents.size();
            const auto texcoordBase = (uint32_t)texCoords0.size();
            const auto indexBase = (uint32_t)indices.size();
            const auto clusterBase = (uint32_t)clusters.size();
            const auto meshBase = (uint32_t)meshes.size();
            const auto instanceBase = (uint32_t)instances.size();

            // copy offset-independent buffers
            positions.insert(positions.end(), src.positions.begin(), src.positions.end());
            normals.insert(normals.end(), src.normals.begin(), src.normals.end());
            tangents.insert(tangents.end(), src.tangents.begin(), src.tangents.end());
            texCoords0.insert(texCoords0.end(), src.texCoords0.begin(), src.texCoords0.end());
            indices.insert(indices.end(), src.indices.begin(), src.indices.end());

            // clusters
            for (ClusterStorage cluster : src.clusters)
            {
                cluster.positionsBaseOffset += positionBase;
                cluster.normalsBaseOffset += normalBase;
                cluster.tangentsBaseOffset += tangentBase;
                cluster.texCoords0BaseOffset += texcoordBase;
                cluster.trianglesBaseOffset += indexBase;
                // im not sure if this should change, but im gonna still using this :p
                cluster.clusterID += clusterBase; // global index¿

                clusters.push_back(cluster);
            }

            // meshes
            for (MeshStorage mesh : src.meshes)
            {
                mesh.clusterOffset += clusterBase;
                meshes.push_back(mesh);
            }

            // instances
            for (InstanceStorage instance : src.instances)
            {
                instance.meshIdx += meshBase;
                instances.push_back(instance);
            }
        }

        // first create memory
        positionTable = GpuUploadVector<float3>(desc.device, positions.size());
        if (normals.size() > 0) normalTable = GpuUploadVector<float3>(desc.device, normals.size());
        if (tangents.size() > 0) tangentTable = GpuUploadVector<float4>(desc.device, tangents.size());
        if (texCoords0.size() > 0) texCoord0Table = GpuUploadVector<float2>(desc.device, texCoords0.size());
        indexTable = GpuUploadVector<uint8_t>(desc.device, indices.size());
        clusterTable = GpuUploadVector<ClusterStorage>(desc.device, clusters.size());
        meshTable = GpuUploadVector<MeshStorage>(desc.device, meshes.size());
        // instance can be created/deleted at runtime, are not fixed like geometry information 
        instanceTable = GpuMappedVector<InstanceStorage>(desc.device, std::max<uint32_t>(desc.maxInstanceCount, instances.size()));

        // next do copies
        positionTable.append(positions);
        if (normals.size() > 0) normalTable.append(normals);
        if (tangents.size() > 0) tangentTable.append(tangents);
        if (texCoords0.size() > 0)  texCoord0Table.append(texCoords0);
        indexTable.append(indices);
        clusterTable.append(clusters);
        meshTable.append(meshes);
        if (instances.size() > 0) instanceTable.append(instances);

        // upload to device memory
        // this could be really improve by multithreading and/or using WaitTokens
        // but this works
        positionTable.Upload(m_cmdAlloc);
        if (normals.size() > 0)  normalTable.Upload(m_cmdAlloc);
        if (tangents.size() > 0)  tangentTable.Upload(m_cmdAlloc);
        if (texCoords0.size() > 0)  texCoord0Table.Upload(m_cmdAlloc);
        indexTable.Upload(m_cmdAlloc);
        clusterTable.Upload(m_cmdAlloc);
        meshTable.Upload(m_cmdAlloc);
        instanceTable.append(instances);
        // instanceTable is already on device memory
    }
    catch (...)
    {
        throw;
    }
}

void SceneView::Destroy()
{
    if (!m_device)
        return;

    m_device->WaitFor(QueueType::Copy);
    m_device->Destroy(m_cmdAlloc);

    m_cmdAlloc = {};
    m_device = nullptr;
}