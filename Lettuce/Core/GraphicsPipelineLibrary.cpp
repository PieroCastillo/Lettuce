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
    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        VkGraphicsPipelineLibraryCreateInfoEXT libraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT,
            .flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT,
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = {

        };

        VkPipelineVertexInputStateCreateInfo vertexInputCI = {

        };

        VkGraphicsPipelineCreateInfo pipelineLibraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->_device, VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
        vertexInputLibraries[name] = pipeline;
        return;
    }
}

void GraphicsPipelineLibrary::AddPreRasterizationShaders(std::string name)
{
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
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->_device, VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
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

        VkGraphicsPipelineCreateInfo pipelineLibraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->_device, VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
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

        VkGraphicsPipelineCreateInfo pipelineLibraryCI = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &libraryCI,
            .flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT,
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->_device, VK_NULL_HANDLE, 1, &pipelineLibraryCI, nullptr, &pipeline));
        fragmentOutputLibraries[name] = pipeline;
        return;
    }
}

void GraphicsPipelineLibrary::Release()
{
    if (_device->GetEnabledRecommendedFeatures().graphicsPipelineLibrary)
    {
        for (auto &pl : vertexInputLibraries)
        {
            vkDestroyPipeline(_device->_device, pl, nullptr);
        }
        for (auto &pl : preRasterizationShadersLibraries)
        {
            vkDestroyPipeline(_device->_device, pl, nullptr);
        }
        for (auto &pl : fragmentShaderLibraries)
        {
            vkDestroyPipeline(_device->_device, pl, nullptr);
        }
        for (auto &pl : fragmentOutputLibraries)
        {
            vkDestroyPipeline(_device->_device, pl, nullptr);
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
            .layout =layout->
        };

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkResult(vkCreateGraphicsPipelines(_device->_device, VK_NULL_HANDLE, 1, &executablePipeline, nullptr, &pipeline));
        return pipeline;
    }
}