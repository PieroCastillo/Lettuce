//
// created by piero on 25/11/2024
//
#include "Lettuce/Core/Common.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <exception>
#include <shaderc/shaderc.hpp>
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Compilers/ICompiler.hpp"
#include "Lettuce/Core/Compilers/SlangCompiler.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Compilers;

void SlangCompiler::Load()
{
    return;
}

std::vector<uint32_t> SlangCompiler::Compile(std::string text, std::string inputFile, PipelineStage stage, bool optimize) 
{
    shaderc_shader_kind kind;

    switch (stage)
    {
    case PipelineStage::Vertex:
        kind = shaderc_vertex_shader;
        break;
    case PipelineStage::TessellationControl:
        kind = shaderc_tess_control_shader;
        break;
    case PipelineStage::TessellationEvaluation:
        kind = shaderc_tess_evaluation_shader;
        break;
    case PipelineStage::Geometry:
        kind = shaderc_geometry_shader;
        break;
    case PipelineStage::Fragment:
        kind = shaderc_fragment_shader;
        break;
    case PipelineStage::Compute:
        kind = shaderc_compute_shader;
        break;
    case PipelineStage::Task:
        kind = shaderc_task_shader;
        break;
    case PipelineStage::Mesh:
        kind = shaderc_mesh_shader;
        break;
    default:
        throw std::exception("shader stage doesn't supported by shaderc");
    }

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    if (optimize)
        options.SetOptimizationLevel(shaderc_optimization_level_size);

    // auto pre = compiler.PreprocessGlsl(text, kind, "shader_src", options);
    try
    {
        shaderc::SpvCompilationResult module =
            compiler.CompileGlslToSpv(text, kind, inputFile.c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            throw std::exception(module.GetErrorMessage().c_str());
        }
        return {module.cbegin(), module.cend()};
    }
    catch (std::exception e)
    {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}

void SlangCompiler::Destroy()
{
    return;
}