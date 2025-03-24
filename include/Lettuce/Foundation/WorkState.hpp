//
// Created by piero on 23/03/2025.
//
#pragma once
#include <vector>
#include <memory>

namespace Lettuce::Foundation
{
    // NEEDS REVIEW

    struct WorkState
    {
        virtual void resordState(VkCommandBuffer cmd) = 0;
    };

    struct BindPipeline;
    struct BindShaders;
    struct BindDescriptorSets;
    struct BindDescriptorBuffers;
    struct BindVertexBuffers;
    struct BindIndexBuffer;

    // required

    struct ShaderState
    {
        uint32_t viewportCount;
        VkViewport *pViewports;
        uint32_t scissorCount;
        VkRect2D *pScissors;
        VkBool32 rasterizerDiscardEnable;
        VkSampleCountFlagBits rasterizationSamples;
        VkSampleCountFlagBits samples;
        VkSampleMask *pSampleMask;
        VkBool32 alphaToCoverageEnable;
        VkBool32 alphaToOneEnable;
        VkPolygonMode polygonMode;
        float lineWidth;
        VkCullModeFlags cullMode;
        VkFrontFace frontFace;
        VkBool32 depthTestEnable;
        VkBool32 depthWriteEnable;
        VkCompareOp depthCompareOp;
        VkBool32 depthBoundsTestEnable;

        float minDepthBounds;
        float maxDepthBounds;

        VkBool32 depthBiasEnable;

        float depthBiasantFactor;
        float depthBiasClamp;
        float depthBiasSlopeFactor;

        VkBool32 stencilTestEnable;

        VkStencilFaceFlags faceMask;
        VkStencilOp failOp;
        VkStencilOp passOp;
        VkStencilOp depthFailOp;
        VkCompareOp compareOp;

        VkStencilFaceFlags compareFaceMask;
        uint32_t compareMask;

        VkStencilFaceFlags writeFaceMask;
        uint32_t writeMask;

        VkStencilFaceFlags referenceFaceMask;
        uint32_t reference;
    };

    // vertex
    struct VertexShaderState
    {
        uint32_t vertexBindingDescriptionCount;
        VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions;
        uint32_t vertexAttributeDescriptionCount;
        VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions;
        VkPrimitiveTopology primitiveTopology;
        VkBool32 primitiveRestartEnable;
    };

    // tessellation eval
    struct TessellationEvaluationShaderState
    {
        uint32_t patchControlPoints
    };

    // tesellation control
    struct TessellationControlShaderState
    {
        VkTessellationDomainOrigin domainOrigin;
    };

    // fragment
    struct FragmentShaderState
    {
        VkBool32 logicOpEnable;
        VkLogicOp logicOp;
        uint32_t firstAttachment;
        uint32_t attachmentCount;
        VkBool32 *pColorBlendEnables;
        VkColorComponentFlags *pColorWriteMasks;
        VkColorBlendEquationEXT *pColorBlendEquations;
        VkColorBlendAdvancedEXT *pColorBlendAdvanced;
        float blendConstants[4];
    };

    // fragment shading rate
    struct FragmentShadingRateState
    {
        VkExtent2D *pFragmentSize;
        VkFragmentShadingRateCombinerOpKHR combinerOps[2];
    };

    // geometry
    struct GeometryShaderState
    {
        uint32_t rasterizationStream;
    };
}