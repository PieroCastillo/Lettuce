//
// created by piero on 25/06/2024
//
module;
#include <vector>
#include <iostream>
#include <string>
#include <exception>
#include <shaderc/shaderc.hpp>

export module Lettuce:GLSLCompiler;

import :ICompiler;
import :PipelineConnector;

export namespace Lettuce::Core::Compilers
{
    class GLSLCompiler : public ICompiler
    {
    private:
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

    public:
        void Load() override
        {
        }

        std::vector<uint32_t> Compile(std::string text, std::string mainMethod, LettucePipelineStage stage, bool optimize = false) override
        {
            shaderc_shader_kind kind;

            switch (stage)
            {
            case LettucePipelineStage::Vertex:
                kind = shaderc_vertex_shader;
                break;
            case LettucePipelineStage::TessellationControl:
                kind = shaderc_tess_control_shader;
                break;
            case LettucePipelineStage::TessellationEvaluation:
                kind = shaderc_tess_evaluation_shader;
                break;
            case LettucePipelineStage::Geometry:
                kind = shaderc_geometry_shader;
                break;
            case LettucePipelineStage::Fragment:
                kind = shaderc_fragment_shader;
                break;
            case LettucePipelineStage::Compute:
                kind = shaderc_compute_shader;
                break;
            case LettucePipelineStage::Task:
                kind = shaderc_task_shader;
                break;
            case LettucePipelineStage::Mesh:
                kind = shaderc_mesh_shader;
                break;
            default:
                throw std::exception("shader stage doesn't supported by shaderc");
            }

            if (optimize)
                options.SetOptimizationLevel(shaderc_optimization_level_size);

            try
            {
                shaderc::SpvCompilationResult module =
                    compiler.CompileGlslToSpv(text, kind, mainMethod.c_str(), options);

                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    throw std::exception(module.GetErrorMessage().c_str());
                }
                return {module.cbegin(), module.cend()};
            }
            catch (std::exception e)
            {
                throw e;
            }
        }

        void Destroy() override
        {
            compiler.~Compiler();
        }
    };
}