// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Rendering/Debug/DebugPass.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Rendering;
using namespace Lettuce::Rendering::Debug;

DebugPass::DebugPass(const DebugPassDesc&)
{

}

DebugPass::~DebugPass()
{

}

DebugPass::DebugPass(DebugPass&& other) noexcept
{

}

DebugPass& DebugPass::operator=(DebugPass&& other) noexcept
{
    return *this;
}

void DebugPass::Record(CommandBuffer& cmd, const DebugPassRecordDesc& desc)
{

}