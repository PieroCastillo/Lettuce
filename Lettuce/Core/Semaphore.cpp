//
// Created by piero on 14/03/2024.
//
#include "Lettuce/Core/Common.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <memory>
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"
#include "Lettuce/Core/Utils.hpp"

using namespace Lettuce::Core;

Semaphore::Semaphore(const std::shared_ptr<Device> &device, uint64_t initialValue) : _device(device)
{
    VkSemaphoreTypeCreateInfo semaphoreTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = initialValue,
    };
    VkSemaphoreCreateInfo semaphoreCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCI,
    };
    checkResult(vkCreateSemaphore(device->GetHandle(), &semaphoreCI, nullptr, GetHandlePtr()));
}
void Semaphore::Wait(uint64_t value)
{
    VkSemaphoreWaitInfo waitI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = GetHandlePtr(),
        .pValues = &value,
    };
    checkResult(vkWaitSemaphores(_device->GetHandle(), &waitI, (std::numeric_limits<uint64_t>::max)()));
}

void Semaphore::Release()
{
    vkDestroySemaphore(_device->GetHandle(), GetHandle(), nullptr);
}

void Semaphore::Signal(uint64_t signalValue)
{
    VkSemaphoreSignalInfo semaphoreSignalI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
        .semaphore = GetHandle(),
        .value = signalValue,
    };
    checkResult(vkSignalSemaphore(_device->GetHandle(), &semaphoreSignalI));
}

std::vector<std::shared_ptr<Semaphore>> Semaphore::CreateSemaphores(const std::shared_ptr<Device> &device, uint32_t semaphoresCount)
{
    std::vector<std::shared_ptr<Semaphore>> semaphores;
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
        semaphores[i]->_device = device;
        checkResult(vkCreateSemaphore(device->GetHandle(), &semaphoreCI, nullptr, semaphores[i]->GetHandlePtr()));
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
                   { return x.GetHandle(); });
    VkSemaphoreWaitInfo semaphoreWaitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = (uint32_t)semaphores.size(),
        .pSemaphores = vkSemaphores.data(),
        .pValues = values.data(),
    };
    checkResult(vkWaitSemaphores(semaphores[0]._device->GetHandle(), &semaphoreWaitInfo, (std::numeric_limits<uint64_t>::max)()));
}

void Semaphore::DestroySemaphores(std::vector<Semaphore> semaphores)
{
    for (int i = 0; i < semaphores.size(); i++)
    {
        vkDestroySemaphore(semaphores[i]._device->GetHandle(), semaphores[i].GetHandle(), nullptr);
    }
}