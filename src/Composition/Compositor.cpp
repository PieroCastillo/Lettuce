// standard headers
#include <algorithm>
#include <atomic>
#include <limits>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

// project headers
#include "Lettuce/Composition/api.hpp"
#include "Lettuce/Composition/CompositorImpl.hpp"
#include "Lettuce/Composition/HelperStructs.hpp"

using namespace Lettuce::Composition;

void Compositor::Create(const CompositorDesc& desc)
{
    impl = new CompositorImpl;
    impl->Create(desc);

    impl->MainLoop();
}

void Compositor::Destroy()
{
    impl->Destroy();
    delete impl;
}

void Compositor::Commit()
{
    /*
    locks commitMutex, waits for compositor is not processing
    then swap queues, register new state, so notifies compositor thread
    */
    std::unique_lock<std::mutex> lock(impl->commitMutex);

    impl->appCv.wait(lock, [this]() {return !impl->compositorIsProcessing; });
    std::swap(impl->appQueue, impl->compositorQueue);
    impl->hasNewCommands = true;
    impl->compositorIsProcessing = true;
    impl->compCv.notify_one();
}

void Compositor::SetDebugName(Visual visual, std::string name)
{
    impl->appQueue.addCommand(OpCode::SetDebugName, visual.get(), {}, name);
}

auto Compositor::GetVisualCount() -> uint32_t
{
    return {};
}

auto Compositor::GetAnimationCount() -> uint32_t
{
    return {};
}