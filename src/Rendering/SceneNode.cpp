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

// auto SceneGraph::CreateNode(const std::shared_ptr<IMaterial>& material) -> SceneNode
// {
//     std::shared_ptr<IMaterial> materialPtr;
//     constexpr int maxRenderableCount = 4096;

//     auto materialDataView = impl->allocator->AllocateMemory(sizeof(material->paramsType) * maxRenderableCount);
//     auto geometryDataView  = impl->allocator->AllocateMemory(sizeof(GeometryGPUData) * maxRenderableCount);
//     MemoryView vertexBuffer = impl->allocator->AllocateMemory(sizeof(material->paramsType) * maxRenderableCount);
//     MemoryView indexBuffer = impl->allocator->AllocateMemory(sizeof(material->paramsType) * maxRenderableCount);

//     std::span<std::any> materialsGPUData;
//     std::span<GeometryGPUData> geometryGPUData;
//     std::span<std::any> vertexGPUData;
//     std::span<uint32_t> indexGPUData;

//     IndirectSetDesc setDesc = {
//         .type  = IndirectType::DrawIndexed,
//         .maxCount = 4096,
//         .userDataSize = 0,
//     };
//     auto indirectSet = impl->device->CreateIndirectSet(setDesc);

//     return impl->nodes.allocate({
//         material,
//         materialDataView,
//         geometryDataView,
//         vertexBuffer,
//         indexBuffer,
//         materialsGPUData,
//         geometryGPUData,
//         vertexGPUData,
//         indexGPUData });
// }

// void SceneGraph::Destroy(SceneNode node)
// {

// }