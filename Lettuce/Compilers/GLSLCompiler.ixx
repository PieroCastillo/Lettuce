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

            shaderc::Compiler compiler;
            shaderc::CompileOptions options;

            if (optimize)
                options.SetOptimizationLevel(shaderc_optimization_level_size);

            auto pre = compiler.PreprocessGlsl(text, kind, "shader_src", options);
            try
            {
                shaderc::SpvCompilationResult module =
                    compiler.CompileGlslToSpv({pre.cbegin(), pre.cend()}, kind, "shader_src", options);

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

        void Destroy() override
        {
            // compiler.~Compiler();
        }
    };
}