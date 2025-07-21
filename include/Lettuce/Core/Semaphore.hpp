/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_SEMAPHORE_HPP
#define LETTUCE_CORE_SEMAPHORE_HPP 

namespace Lettuce::Core
{
    struct SemaphoreCreateInfo
    {
        uint64_t initialValue;
    };

    class Semaphore
    {
    private:
    
    public:
        VkDevice m_device;

        Semaphore(VkDevice device, const SemaphoreCreateInfo& createInfo);
        void Release();

        void Wait(uint64_t value);
        void Signal(uint64_t signalValue);
    };
}
#endif // LETTUCE_CORE_SEMAPHORE_HPP