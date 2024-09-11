//
// Created by piero on 14/03/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "Device.hpp"

namespace Lettuce::Core
{
    class Semaphore
    {
    public:
        Device _device;
        VkSemaphore _semaphore;
        // uint64_t currentValue;

        void Create(Device &device, uint64_t initialValue);
        void Wait(uint64_t value);
        void Signal(uint64_t signalValue);
        void Destroy();
        static std::vector<Semaphore> CreateSemaphores(Device &device, uint32_t semaphoresCount);
        static void WaitSemaphores(std::vector<Semaphore> semaphores, std::vector<uint64_t> values);
        static void DestroySemaphores(std::vector<Semaphore> semaphores);
    };
}