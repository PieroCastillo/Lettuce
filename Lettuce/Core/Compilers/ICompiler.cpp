//
// created by piero on 25/06/2024
//
#include <string>
#include <vector>

namespace Lettuce::Core::Compilers
{
    class ICompiler
    {
    public:
        virtual void Load() = 0;
        virtual std::vector<uint32_t> Compile(std::string text, std::string inputFile, PipelineStage stage, bool optimize) = 0;
        virtual void Destroy() = 0;
    };
}