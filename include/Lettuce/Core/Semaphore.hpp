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
        std::shared_ptr<Device> _device;
        VkSemaphore _semaphore;
        // uint64_t currentValue;

        Semaphore(const std::shared_ptr<Device> &device, uint64_t initialValue);
        ~Semaphore();
        void Wait(uint64_t value);
        void Signal(uint64_t signalValue);
        static std::vector<Semaphore> CreateSemaphores(const std::shared_ptr<Device> &device, uint32_t semaphoresCount);
        static void WaitSemaphores(std::vector<Semaphore> semaphores, std::vector<uint64_t> values);
        static void DestroySemaphores(std::vector<Semaphore> semaphores);
    };
}