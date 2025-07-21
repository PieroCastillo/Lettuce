/*
Creted by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_SEMAPHORE_HPP
#define LETTUCE_CORE_SEMAPHORE_HPP 

namespace Lettuce::Core
{
    class Semaphore
    {
    private:
    
    public:
        VkDevice m_device;

        Semaphore(const std::shared_ptr<Device> &device, uint64_t initialValue);
        void Release();

        void Wait(uint64_t value);
        void Signal(uint64_t signalValue);
    };
}
#endif // LETTUCE_CORE_SEMAPHORE_HPP