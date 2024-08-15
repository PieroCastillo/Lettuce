//
// Created by piero on 14/02/2024.
//
#include <iostream>
#include <volk.h>
#include <vector>
#include <string>

namespace Lettuce::Core
{
    class Shader
    {
    public:
        Device _device;
        PipelineStage _stage;
        VkShaderModule _shaderModule;
        std::string _name;

        template <class T = Compilers::Compiler>
        void Create(Device &device, T &compiler, std::string text, std::string mainMethod, std::string inputFile, PipelineStage stage, bool optimize = false);

        void Destroy();
    };
}