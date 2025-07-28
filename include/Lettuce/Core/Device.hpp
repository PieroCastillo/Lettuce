/*
Created by @PieroCastillo on 2025-07-20
*/
#ifndef LETTUCE_CORE_DEVICE_HPP
#define LETTUCE_CORE_DEVICE_HPP

namespace Lettuce::Core
{
    struct Features
    {
        bool FragmentShadingRate;
        bool ExecutionGraphs;
        bool MeshShading;
        bool RayTracing;
        bool Video;
    };

    struct EnabledRecommendedFeatures
    {
        bool shaderObject = false;
        bool deviceGeneratedCommands = false;
        bool graphicsPipelineLibrary = false;
        bool dynamicRenderingLocalRead = false;
        bool maintenance5 = false;
    };

    enum class LettuceResult
    {
        OutOfDeviceMemory,
        OutOfHostMemory,
        Unknown,
    };

    class Device : public IReleasable, public IManageHandle<VkDevice>
    {
    private:
        std::vector<std::string> availableExtensionsNames;
        std::vector<char *> availableLayersNames;
        std::vector<const char *> requestedExtensionsNames;
        std::vector<const char *> requestedLayersNames;
        void *next;
        Features _features;
        Features _enabledFeatures;
        EnabledRecommendedFeatures enabledRecommendedFeatures;

        // physical device features structs
        // required features/extensions
        VkPhysicalDeviceVulkan11Features gpuFeatures11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        };
        VkPhysicalDeviceVulkan12Features gpuFeatures12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &gpuFeatures11,
        };
        VkPhysicalDeviceVulkan13Features gpuFeatures13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &gpuFeatures12,
        };
        VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
            .pNext = &gpuFeatures13,
        };
        VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamicRenderingUnusedAttachmentsFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT,
            .pNext = &descriptorBufferFeature,
        };
        // recommended features/extensions
        VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
        };

        VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT deviceGeneratedCommandsFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT,
        };

        VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphicsPipelineLibraryFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
        };

        VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicRenderingLocalReadFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
        };

        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Feature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
        };

        // optional features/extensions
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
        };

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        };

    public:
        VkInstance m_instance;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        Device();
        void Release();

        EnabledRecommendedFeatures GetEnabledRecommendedFeatures()
        {
            return enabledRecommendedFeatures;
        }

        auto CreateGraphicsPipeline() -> std::expected<std::shared_ptr<Pipeline>, LettuceResult>;
        auto CreateComputePipeline() -> std::expected<std::shared_ptr<Pipeline>, LettuceResult>;
        auto CreateBuffer() -> std::expected<std::shared_ptr<Buffer>, LettuceResult>;
        auto CreateSwapchain() -> std::expected<std::shared_ptr<Swapchain>, LettuceResult>;
        auto CreateSampler() -> std::expected<std::shared_ptr<Sampler>, LettuceResult>;
        auto CreateSemaphore() -> std::expected<std::shared_ptr<Semaphore>, LettuceResult>;
        auto CreateTextureArray() -> std::expected<std::shared_ptr<TextureArray>, LettuceResult>;
        auto CreateTextureView() -> std::expected<std::shared_ptr<TextureView>, LettuceResult>;
        auto CreateRenderFlowGraph() -> std::expected<std::shared_ptr<RenderFlowGraph>, LettuceResult>;
        auto CreateSequentialRenderFlow() -> std::expected<std::shared_ptr<SequentialRenderFlow>, LettuceResult>;
        auto CreateDescriptorTable() -> std::expected<std::shared_ptr<DescriptorTable>, LettuceResult>;
        auto CreateRenderTarget() -> std::expected<std::shared_ptr<RenderTarget>, LettuceResult>;

        void BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<Buffer> buffer);
        void BindMemory(std::shared_ptr<Memory> memory, std::shared_ptr<TextureArray> textureArray);
        
        auto MemoryAlloc(uint32_t size = (16 * 1024 * 1024)) -> std::expected<std::shared_ptr<Memory>, LettuceResult>; // 16 MiB default size
        void MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<Buffer> dstBuffer);
        void MemoryCopy(std::shared_ptr<Buffer> srcBuffer, std::shared_ptr<TextureArray> dstTextureArray);
        void MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<Buffer> dstBuffer);
        void MemoryCopy(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray);

        void Blit(std::shared_ptr<TextureArray> srcTextureArray, std::shared_ptr<TextureArray> dstTextureArray);
    };
}
#endif // LETTUCE_CORE_DEVICE_HPP