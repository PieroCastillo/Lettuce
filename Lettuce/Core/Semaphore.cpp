//
// Created by piero on 14/03/2024.
//
#include "Lettuce/Core/common.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

void Semaphore::Create(const std::shared_ptr<Device> &device, uint64_t initialValue)
{
    _device = device;
    VkSemaphoreTypeCreateInfo semaphoreTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = initialValue,
    };
    VkSemaphoreCreateInfo semaphoreCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCI,
    };
    checkResult(vkCreateSemaphore(device->_device, &semaphoreCI, nullptr, &_semaphore));
    
}
void Semaphore::Wait(uint64_t value)
{
    VkSemaphoreWaitInfo waitI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &_semaphore,
        .pValues = &value,
    };
    checkResult(vkWaitSemaphores(_device->_device, &waitI, (std::numeric_limits<uint64_t>::max)()));
}

void Semaphore::Destroy()
{
    vkDestroySemaphore(_device->_device, _semaphore, nullptr);
}

void Semaphore::Signal(uint64_t signalValue)
{
    VkSemaphoreSignalInfo semaphoreSignalI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
        .semaphore = _semaphore,
        .value = signalValue,
    };
    checkResult(vkSignalSemaphore(_device->_device, &semaphoreSignalI));
}

std::vector<Semaphore> Semaphore::CreateSemaphores(const std::shared_ptr<Device> &device, uint32_t semaphoresCount)
{
    std::vector<Semaphore> semaphores;
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
        checkResult(vkCreateSemaphore(device->_device, &semaphoreCI, nullptr, &(semaphores[i]._semaphore)), "semaphore #" + std::to_string(i) + " created succesfully");
    }
    return semaphores;
}

void Semaphore::WaitSemaphores(std::vector<Semaphore> semaphores, std::vector<uint64_t> values)
{
    if (semaphores.size() != values.size())
    {
        throw std::exception("semaphores and values must have the same size");
    }
    std::vector<VkSemaphore> vkSemaphores;
    std::transform(semaphores.begin(), semaphores.end(), std::back_inserter(vkSemaphores), [](Semaphore x)
                   { return x._semaphore; });
    VkSemaphoreWaitInfo semaphoreWaitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = (uint32_t)semaphores.size(),
        .pSemaphores = vkSemaphores.data(),
        .pValues = values.data(),
    };
    checkResult(vkWaitSemaphores(semaphores[0]._device->_device, &semaphoreWaitInfo, (std::numeric_limits<uint64_t>::max)()));
}

void Semaphore::DestroySemaphores(std::vector<Semaphore> semaphores)
{
    for (int i = 0; i < semaphores.size(); i++)
    {
        vkDestroySemaphore(semaphores[i]._device->_device, semaphores[i]._semaphore, nullptr);
    }
}