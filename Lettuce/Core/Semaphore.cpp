//
// Created by piero on 14/03/2024.
//
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <volk.h>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"

using namespace Lettuce::Core;

std::vector<TSemaphore> TSemaphore::Create(Device &device, uint32_t semaphoresCount)
{
    std::vector<TSemaphore> semaphores;
    semaphores.resize(semaphoresCount);

    // uses timeline semaphores
    VkSemaphoreTypeCreateInfo semaphoreTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
    };
    VkSemaphoreCreateInfo semaphoreCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCI,
    };
    for (int i = 0; i < semaphoresCount; i++)
    {
        semaphores[i]._device = device;
        checkResult(vkCreateSemaphore(device._device, &semaphoreCI, nullptr, &(semaphores[i]._semaphore)), "semaphore #" + std::to_string(i) + " created succesfully");
    }
    return semaphores;
}

void TSemaphore::Wait(std::vector<TSemaphore> semaphores, std::vector<uint64_t> values)
{
    if (semaphores.size() != values.size())
    {
        throw std::exception("semaphores and values must have the same size");
    }
    std::vector<VkSemaphore> vkSemaphores;
    std::transform(semaphores.begin(), semaphores.end(), std::back_inserter(vkSemaphores), [](TSemaphore x)
                   { return x._semaphore; });
    VkSemaphoreWaitInfo semaphoreWaitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = (uint32_t)semaphores.size(),
        .pSemaphores = vkSemaphores.data(),
        .pValues = values.data(),
    };
    checkResult(vkWaitSemaphores(semaphores[0]._device._device, &semaphoreWaitInfo, std::numeric_limits<uint64_t>::max()));
}

void TSemaphore::Signal(uint64_t signalValue)
{
    VkSemaphoreSignalInfo semaphoreSignalI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
        .semaphore = _semaphore,
        .value = signalValue,
    };
    checkResult(vkSignalSemaphore(_device._device, &semaphoreSignalI));
}

void TSemaphore::Destroy(std::vector<TSemaphore> semaphores)
{
    for (int i = 0; i < semaphores.size(); i++)
    {
        vkDestroySemaphore(semaphores[i]._device._device, semaphores[i]._semaphore, nullptr);
    }
}

void BSemaphore::Create(Device &device)
{
    _device = device;

    VkSemaphoreTypeCreateInfo semaphoreTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_BINARY,
    };
    VkSemaphoreCreateInfo semaphoreCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCI,
    };
    checkResult(vkCreateSemaphore(_device._device, &semaphoreCI, nullptr, &_semaphore));
}

void BSemaphore::Destroy()
{
    vkDestroySemaphore(_device._device, _semaphore, nullptr);
}