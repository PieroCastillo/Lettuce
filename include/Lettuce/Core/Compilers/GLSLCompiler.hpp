//
// created by piero on 25/06/2024
//
#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <exception>
#include <shaderc/shaderc.hpp>

namespace Lettuce::Core::Compilers
{
    class GLSLCompiler : public ICompiler
    {
    public:
        void Load() override = 0;
        std::vector<uint32_t> Compile(std::string text, std::string inputFile, PipelineStage stage, bool optimize) override = 0;
        void Destroy() override = 0;
    };
}