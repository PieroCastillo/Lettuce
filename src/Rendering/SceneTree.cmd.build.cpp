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

/*
build  cblas: dispatch( ceil(clasCount/threadPerGroup) ) per build
build  ptlas: dispatch( ceil(instanceCount/threadPerGroup) ) per build
update ptlas: dispatch( ceil(instanceCount/threadPerGroup) ) per build
*/

void SceneTreeCommandBuffer::Build(const BuildGeometryClusterDesc& build)
{

}

void SceneTreeCommandBuffer::Build(const BuildScenePartitionDesc& build)
{

}