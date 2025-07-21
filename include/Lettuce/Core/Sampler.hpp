/*
Creted by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_SAMPLER_HPP
#define LETTUCE_CORE_SAMPLER_HPP 

namespace Lettuce::Core
{
    class Sampler
    {
    private:
    public:
        VkDevice m_device;
        VkSampler m_sampler;

        Sampler(VkDevice device);
        void Release();
    };
}
#endif // LETTUCE_CORE_SAMPLER_HPP