// standard headers
#include <memory>
#include <memory_resource>
#include <span>
#include <vector>

// project headers
#include "Lettuce/helper.hpp"
#include "Lettuce/Core/api.hpp"
#include "Lettuce/Quimera/SurfaceImpl.hpp"
#include "Lettuce/Quimera/api.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::Core;

void SurfaceCommandBuffer::DrawSurface()
{
    cmd->BindDescriptorTable(surfPtr->impl->dtSurface, PipelineBindPoint::Compute);
    uint32_t cmdCount = 0;
    uint32_t tileXCount = (1920 + 15) / 16;
    uint32_t tileYCount = (1080 + 15) / 16;
    uint32_t threadGroupCount = 32;
    
    cmd->Dispatch(ceil(cmdCount / threadGroupCount), 1, 1);

    cmd->Dispatch(tileXCount, tileYCount, 1);

    cmd->Dispatch(tileXCount, tileYCount, 1);
}