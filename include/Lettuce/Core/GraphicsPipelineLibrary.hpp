//
// Created by piero on 9/04/2024.
//
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "ShaderModule.hpp"
#include "Utils.hpp"
#include "PipelineLayout.hpp"

namespace Lettuce::Core
{
    class GraphicsPipelineLibrary : public IReleasable
    {
    private:
        // main opt: VK_EXT_graphics_pipeline_library
        std::map<std::string, VkPipeline> vertexInputLibraries;
        std::map<std::string, VkPipeline> preRasterizationShadersLibraries;
        std::map<std::string, VkPipeline> fragmentShaderLibraries;
        std::map<std::string, VkPipeline> fragmentOutputLibraries;

        // emulation: store states

    public:
        std::shared_ptr<Device> _device;
        
        void AddVertexInput(std::string name);
        void AddPreRasterizationShaders(std::string name);
        void AddFragmentShader(std::string name);
        void AddFragmentOutput(std::string name);

        GraphicsPipelineLibrary(const std::shared_ptr<Device> &device) : _device(device)
        {
        }

        void Release();

        VkPipeline AssemblyPipeline(std::string vertexInputName,
                                    std::string preRasterizationShadersName,
                                    std::string fragmentShaderName,
                                    std::string fragmentOutputName,
                                    const std::shared_ptr<PipelineLayout> &layout);
    };
}