/*
Created by @PieroCastillo on 2026-05-17
*/
#ifndef LETTUCE_RENDERING_SCENE_VIEW_HPP
#define LETTUCE_RENDERING_SCENE_VIEW_HPP

#include "../Core/api.hpp"
#include "../Foundations/api.hpp"
#include "types.hpp"

using namespace Lettuce::Foundations;

namespace Lettuce::Rendering
{
    struct SceneViewDesc
    {
        Device& device;
        std::span<GeometrySource> sources;
        uint32_t maxInstanceCount;
    };

    class SceneView
    {
    private:
        Device* m_device = nullptr;
        CommandAllocator m_cmdAlloc;

        // vertex streams
        GpuUploadVector<float3> positionTable;
        GpuUploadVector<float3> normalTable;
        GpuUploadVector<float4> tangentTable;
        GpuUploadVector<float2> texCoord0Table;

        // data over vertices
        GpuUploadVector<uint8_t> indexTable;
        GpuUploadVector<ClusterStorage> clusterTable;
        GpuUploadVector<MeshStorage> meshTable;
        GpuMappedVector<InstanceStorage> instanceTable;
    public:
        SceneView() noexcept = default;
        explicit SceneView(const SceneViewDesc& desc);

        ~SceneView();

        SceneView(const SceneView&) = delete;
        SceneView& operator=(const SceneView&) = delete;

        SceneView(SceneView&&) noexcept;
        SceneView& operator=(SceneView&&) noexcept;

        void Create(const SceneViewDesc&);
        void Destroy();

        auto GetPositionsView() { return positionTable.getView(); }
        auto GetNormalsView() { return normalTable.getView(); }
        auto GetTangentsView() { return tangentTable.getView(); }
        auto GetTexCoords0View() { return texCoord0Table.getView(); }

        auto GetIndicesView() { return indexTable.getView(); }
        auto GetMeshesView() { return meshTable.getView(); }
        auto GetClustersView() { return clusterTable.getView(); }
        auto& GetInstanceTable() { return instanceTable; }
    };
};
#endif // LETTUCE_RENDERING_SCENE_VIEW_HPP