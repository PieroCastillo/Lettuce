//
// Created by piero on 14/03/2024.
//
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <volk.h>

namespace Lettuce::Core
{
    class TSemaphore
    {
    public:
        Device _device;
        VkSemaphore _semaphore;

        static std::vector<TSemaphore> Create(Device &device, uint32_t semaphoresCount);

        static void Wait(std::vector<TSemaphore> semaphores, std::vector<uint64_t> values);

        void Signal(uint64_t signalValue);

        static void Destroy(std::vector<TSemaphore> semaphores);
    };

    class BSemaphore
    {
    public:
        Device _device;
        VkSemaphore _semaphore;

        void Create(Device &device);

        void Destroy();
    };
}