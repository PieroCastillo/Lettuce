// standard headers
#include <functional>
#include <vector>
#include <mutex>

// external headers
#include <volk.h>

// project headers
#include "Lettuce/Core/DeviceExecutionContext.hpp"

using namespace Lettuce::Core;

void DeviceThreadPool::Create(uint32_t threadCount)
{
    for (uint32_t i = 0; i < threadCount; ++i)
    {
        workers.emplace_back([this]
            {
                while (true)
                {
                    std::function<void()> task;
                    // check access to queue & get task
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);

                        cv.wait(lock, [this]
                            {
                                return stop || !tasks.empty();
                            });
                        if (stop && tasks.empty())
                        {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
    }
}

void DeviceThreadPool::Release()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = false;
    }
    cv.notify_all();

    for (auto& thread : workers)
    {
        thread.join();
    }
}