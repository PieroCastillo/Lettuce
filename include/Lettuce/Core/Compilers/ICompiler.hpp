//
// created by piero on 25/06/2024
//
#pragma once
#include <string>
#include <vector>

namespace Lettuce::Core::Compilers
{
    class ICompiler
    {
    public:
        virtual void Load() {};
        virtual std::vector<uint32_t> Compile(std::string text, std::string inputFile, PipelineStage stage, bool optimize) {return {};};
        virtual void Destroy() {};
    };
}