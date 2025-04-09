//
// Created by piero on 7/04/2025.
//
#include <vector>

#include "Material.hpp"
#include "MaterialFamily.hpp"

#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"

namespace Lettuce::Geometry::Materials
{
    class MaterialLibrary
    {
        // internal data
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPipelineLayout> pipelineLayouts;
        std::vector<VkPipeline> pipelines;
        std::vector<VkShaderModule> shaderModules;
        uint32_t descriptorBufferSize;

        std::vector<MaterialFamily> families;

        uint32_t GetDescriptorBufferAllocationSize();
        void SetDescriptorBufferOffset(std::shared_ptr<BufferResource> descriptorBuffer, uint32_t offset);

        virtual void AddFamilies();
        virtual void AddMaterials();
        
        MaterialFamily GetByName(std::string familyName);
        Material GetByName(std::string familyName, std::string materialName);

        void BindLibrary(VkCommandBuffer cmd);
        void BindFamily(VkCommandBuffer cmd, MaterialFamily family);
        void BindMaterial(VkCommandBuffer cmd, Material material);
    };
}