//
// created by piero on 25/06/2024
//
module;
#include <string>
#include <vector>

export module Lettuce:ICompiler;

import :PipelineConnector;

export namespace Lettuce::Core::Compilers
{
    class ICompiler
    {
    public:
        virtual void Load() = 0;
        virtual std::vector<uint32_t> Compile(std::string text, std::string mainMethod, LettucePipelineStage stage, bool optimize) = 0;
        virtual void Destroy() = 0;
    };
}