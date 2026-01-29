/*
Created by @PieroCastillo on 2025-01-5
*/
#ifndef LETTUCE_RENDERING_API_HPP
#define LETTUCE_RENDERING_API_HPP

#include <any>
#include <array>
#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <span>
#include <thread>
#include <vector>

// project headers
#include "../Core/api.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Rendering
{
    struct AsyncRecorderDesc
    {
        Device& device;
        uint32_t threadCount;
    };

    struct AsyncRecorder
    {
    private:
        struct Task
        {
            uint32_t taskLevel;
            std::any userData;
            std::move_only_function<void(CommandBuffer, std::any)> recordFunc;
        };

        Device* device;
        std::vector<std::jthread> threads;
        std::vector<CommandAllocator> allocators;

        // synchronization objects
        std::atomic<bool> stop;
        std::mutex tasksMutex;
        std::queue<Task> taskQueue;
        std::condition_variable taskCV;

        std::mutex cmdsMutex;
        std::vector<std::vector<CommandBuffer>> cmdLevels;

        std::atomic<uint32_t> pendingTasks;
        std::atomic<uint32_t> currentLevel;
    public:
        void Create(const AsyncRecorderDesc& desc);
        void Destroy();

        void Reset();

        void RecordAsync(std::any, std::move_only_function<void(CommandBuffer, std::any)>&&);
        void Barrier();

        void Submit(std::optional<Swapchain> optSwapchain = std::nullopt);
    };
};
#endif // LETTUCE_RENDERING_API_HPP