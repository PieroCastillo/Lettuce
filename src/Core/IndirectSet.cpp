// standard headers
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Core/DeviceImpl.hpp"
#include "Lettuce/Core/common.hpp"

using namespace Lettuce::Core;

IndirectSet Device::CreateIndirectSet(const IndirectSetDesc& desc)
{
    // TODO: impl
    return impl->indirectSets.allocate({});
}

void Device::Destroy(IndirectSet indirectSet)
{

}