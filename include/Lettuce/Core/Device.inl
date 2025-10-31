/*
Created by @PieroCastillo on 2025-10-31
*/
#ifndef LETTUCE_CORE_DEVICE_INL
#define LETTUCE_CORE_DEVICE_INL

#include "Device.hpp"

using namespace Lettuce::Core;

template<ICommandRecordingContext... Contexts>
auto Device::CreateGraph() -> Result<RenderFlowGraph<Contexts...>>
{
    try
    {
        auto graph = std::make_shared<RenderFlowGraph<Contexts...>>();
        RenderFlowGraphCreateInfo ci = {

        };
        graph->Create(*this, ci);
        return graph;
    }
    catch(LettuceException e)
    {
        return std::unexpected(e.result);
    }
    catch(...)
    {
        return std::unexpected(LettuceResult::Unknown);
    }
}
#endif // LETTUCE_CORE_DEVICE_INL