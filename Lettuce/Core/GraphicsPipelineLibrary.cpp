//
// Created by piero on 9/04/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "Lettuce/Core/Common.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/IReleasable.hpp"
#include "Lettuce/Core/ShaderModule.hpp"
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/GraphicsPipelineLibrary.hpp"

using namespace Lettuce::Core;

void GraphicsPipelineLibrary::AddVertexInput(std::string name)
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = {

    };

    VkPipelineVertexInputStateCreateInfo vertexInputCI = {

    };

    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        VkGraphicsPipelineLibraryCreateInfoEXT libraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT,
            .flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT,
        };

        VkGraphicsPipelineCreateInfo pipelineLibraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT,
            .pVertexInputState = &vertexInputCI,
            .pInputAssemblyState = &inputAssemblyCI,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->GetHandle(), VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
        vertexInputLibraries[name] = pipeline;
        return;
    }
}

void GraphicsPipelineLibrary::AddPreRasterizationShaders(std::string name)
{
    // VkPipelineShaderStageCreateInfo
    // VkPipelineViewportStateCreateInfo , dynamic
    std::vector<VkPipelineShaderStageCreateInfo> shaders;
    VkPipelineRasterizationStateCreateInfo rasterizationState = {

    };
    VkPipelineTessellationStateCreateInfo tessState = {

    };

    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        VkGraphicsPipelineLibraryCreateInfoEXT libraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT,
            .flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT,
        };

        VkGraphicsPipelineCreateInfo pipelineLibraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT,
            .stageCount = (uint32_t)shaders.size(),
            .pStages = shaders.data(),
            .pTessellationState = &tessState,
            .pRasterizationState = &rasterizationState,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->GetHandle(), VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
        preRasterizationShadersLibraries[name] = pipeline;
        return;
    }
}
void GraphicsPipelineLibrary::AddFragmentShader(std::string name)
{
    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        VkGraphicsPipelineLibraryCreateInfoEXT libraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT,
            .flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT,
        };

        VkPipelineShaderStageCreateInfo stage = {

        };
        VkPipelineMultisampleStateCreateInfo multisampleState = {

        };
        VkPipelineDepthStencilStateCreateInfo depthStencilState = {

        };

        VkGraphicsPipelineCreateInfo pipelineLibraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT,
            .stageCount = 1,
            .pStages = &stage,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = &depthStencilState,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->GetHandle(), VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
        fragmentShaderLibraries[name] = pipeline;
        return;
    }
}
void GraphicsPipelineLibrary::AddFragmentOutput(std::string name)
{
    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        VkGraphicsPipelineLibraryCreateInfoEXT libraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT,
            .flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT,
        };

        VkPipelineColorBlendStateCreateInfo colorBlendState = {

        };
        // VkPipelineMultisampleStateCreateInfo
        // VkPipelineRenderingCreateInfo

        VkGraphicsPipelineCreateInfo pipelineLibraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT,
            .pColorBlendState = &colorBlendState,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->GetHandle(), VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
        fragmentOutputLibraries[name] = pipeline;
        return;
    }
}

void GraphicsPipelineLibrary::Release()
{
    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        for (auto &[_, pl] : vertexInputLibraries)
        {
            vkDestroyPipeline(_device->GetHandle(), pl, nullptr);
        }
        for (auto &[_, pl] : preRasterizationShadersLibraries)
        {
            vkDestroyPipeline(_device->GetHandle(), pl, nullptr);
        }
        for (auto &[_, pl] : fragmentShaderLibraries)
        {
            vkDestroyPipeline(_device->GetHandle(), pl, nullptr);
        }
        for (auto &[_, pl] : fragmentOutputLibraries)
        {
            vkDestroyPipeline(_device->GetHandle(), pl, nullptr);
        }
        return;
    }
}

VkPipeline GraphicsPipelineLibrary::AssemblyPipeline(std::string vertexInputName,
                                                     std::string preRasterizationShadersName,
                                                     std::string fragmentShaderName,
                                                     std::string fragmentOutputName,
                                                     const std::shared_ptr<PipelineLayout> &layout)
{
    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        VkPipeline libraries[] = {
            vertexInputLibraries[vertexInputName],
            preRasterizationShadersLibraries[preRasterizationShadersName],
            fragmentShaderLibraries[fragmentShaderName],
            fragmentOutputLibraries[fragmentOutputName],
        };

        VkPipelineLibraryCreateInfoKHR libraryCI = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR,
            .libraryCount = 4,
            .pLibraries = libraries,
        };

        VkGraphicsPipelineCreateInfo executablePipeline = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT,
            .layout = layout->GetHandle(),
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->GetHandle(), VK_NULL_HANDLE, 1, &executablePipeline, nullptr, &pipeline));
        return pipeline;
    }
}