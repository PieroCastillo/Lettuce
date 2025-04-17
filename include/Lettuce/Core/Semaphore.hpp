//
// Created by piero on 14/03/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "IManageHandle.hpp"

namespace Lettuce::Core
{
    class Semaphore : public IReleasable, public IManageHandle<VkSemaphore>
    {
    public:
        std::shared_ptr<Device> _device;
        VkSemaphore _semaphore;

        Semaphore(const std::shared_ptr<Device> &device, uint64_t initialValue);
        void Release();
        void Wait(uint64_t value);
        void Signal(uint64_t signalValue);
        static std::vector<std::shared_ptr<Semaphore>> CreateSemaphores(const std::shared_ptr<Device> &device, uint32_t semaphoresCount);
        static void WaitSemaphores(std::vector<Semaphore> semaphores, std::vector<uint64_t> values);
        static void DestroySemaphores(std::vector<Semaphore> semaphores);
    };
}