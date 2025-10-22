// project headers
#include "Lettuce/Core/FrameContext.hpp"

using namespace Lettuce::Core;

void FrameContext::Create(const IDevice& device, const FrameContextCreateInfo& createInfo)
{
    m_device = device.m_device;

    VkSemaphoreCreateInfo semaphoreCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    handleResult(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &m_presentSemaphore));

    VkSemaphoreTypeCreateInfo semaphoreTypeCI = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0,
    };
    semaphoreCI.pNext = &semaphoreTypeCI;

    m_semaphores.reserve(createInfo.semaphoreCount);
    for(int i = 0; i < createInfo.semaphoreCount; ++i)
    {
        VkSemaphore semaphore;
        handleResult(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
        m_semaphores.push_back(semaphore);
    }
}

void FrameContext::Release()
{
    for(const auto& semaphore : m_semaphores)
    {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }
    m_semaphores.clear();
    vkDestroySemaphore(m_device, m_presentSemaphore, nullptr);
}