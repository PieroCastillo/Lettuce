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

void SceneView::Create(const SceneViewDesc& desc)
{
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
    normalTable = GpuUploadVector<float3>(desc.device, normals.size());
    tangentTable = GpuUploadVector<float4>(desc.device, tangents.size());
    texCoord0Table = GpuUploadVector<float2>(desc.device, texCoords0.size());
    indexTable = GpuUploadVector<uint8_t>(desc.device, indices.size());
    clusterTable = GpuUploadVector<ClusterStorage>(desc.device, clusters.size());
    meshTable = GpuUploadVector<MeshStorage>(desc.device, meshes.size());
    instanceTable = GpuMappedVector<InstanceStorage>(desc.device, instances.size());

    // next do copies
    positionTable.append(positions);
    normalTable.append(normals);
    tangentTable.append(tangents);
    texCoord0Table.append(texCoords0);
    indexTable.append(indices);
    clusterTable.append(clusters);
    meshTable.append(meshes);
    instanceTable.append(instances);

    // upload to device memory
    // this could be really improve by multithreading and/or using WaitTokens
    // but this works
    positionTable.Upload(m_cmdAlloc);
    normalTable.Upload(m_cmdAlloc);
    tangentTable.Upload(m_cmdAlloc);
    texCoord0Table.Upload(m_cmdAlloc);
    indexTable.Upload(m_cmdAlloc);
    clusterTable.Upload(m_cmdAlloc);
    meshTable.Upload(m_cmdAlloc);
    // instanceTable is already on device memory
}

void SceneView::Destroy()
{
    m_device->Destroy(m_cmdAlloc);
}