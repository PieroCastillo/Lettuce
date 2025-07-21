/*
Creted by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_PIPELINE_LAYOUT_HPP
#define LETTUCE_CORE_PIPELINE_LAYOUT_HPP 

namespace Lettuce::Core
{
    class PipelineLayout
    {
    private:

    public:
        VkDevice m_device;
            
        PipelineLayout(const std::shared_ptr<Device> &device);
        void Release();

        void AddPushConstant(uint32_t size, VkShaderStageFlags stages);
        void Assemble();
    };
}
#endif // LETTUCE_CORE_PIPELINE_LAYOUT_HPP