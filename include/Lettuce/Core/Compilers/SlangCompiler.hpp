//
// created by piero on 25/11/2024
//
#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <exception>
#include "ICompiler.hpp"

namespace Lettuce::Core::Compilers
{
    class SlangCompiler : public ICompiler
    {
    public:
        void Load();
        std::vector<uint32_t> Compile(std::string text, std::string inputFile, PipelineStage stage, bool optimize);
        void Destroy();
    };
}