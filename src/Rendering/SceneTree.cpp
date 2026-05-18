// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/SceneTree.hpp"
#include "Lettuce/Rendering/SceneTreeImpl.hpp"

using namespace Lettuce::Rendering;
using namespace Lettuce::Core;

void SceneTree::Create(const SceneTreeDesc& desc)
{
    impl = new SceneTreeImpl;
    impl->Create(desc);
}

void SceneTree::Destroy()
{
    impl->Destroy();
    delete impl;
}

auto SceneTree::CreateTemplate(const GeometryClusterTemplateDesc& desc) -> GeometryClusterTemplate
{
    return {};
}

auto SceneTree::CreateInstance(const GeometryClusterInstanceDesc& desc) -> GeometryClusterInstance
{
    return {};
}

auto SceneTree::CreatePartition(const ScenePartitionDesc& desc) -> ScenePartition
{
    return {};
}

auto SceneTree::GetPartitionAddress(ScenePartition partition) const -> DeviceAddress
{
    return {};
}

auto SceneTree::GetTemplateAddress(GeometryClusterTemplate templateHandle) const -> DeviceAddress
{
    return {};
}