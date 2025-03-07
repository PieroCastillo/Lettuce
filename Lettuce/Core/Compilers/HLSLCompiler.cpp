//
// created by piero on 25/11/2024
//
#include "Lettuce/Core/common.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <exception>
#include <cwchar>
#include "Lettuce/Core/Utils.hpp"
#include "Lettuce/Core/Compilers/ICompiler.hpp"
#include "Lettuce/Core/Compilers/HLSLCompiler.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Core::Compilers;

void HLSLCompiler::Load()
{
    // load dxc lib
    if (FAILED(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library))))
    {
        throw std::runtime_error("Could noat init dxc library");
    }

    if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
    {
        throw std::runtime_error("Could noat init dxc compiler");
    }

    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))))
    {
        throw std::runtime_error("Could noat init dxc utils");
    }

    return;
}

std::vector<uint32_t> HLSLCompiler::Compile(std::string text, std::string inputFile, PipelineStage stage, bool optimize)
{
    uint32_t codePage = DXC_CP_ACP;
    CComPtr<IDxcBlobEncoding> blob;
    if (FAILED(utils->CreateBlob(text.c_str(), text.size(), codePage, &blob)))
    {
        throw std::runtime_error("Could not load shader text");
    }
    LPCWSTR targetProfile{};

    switch (stage)
    {
    case PipelineStage::Vertex:
        targetProfile = L"vs_6_7";
        break;
    case PipelineStage::TessellationControl:
        targetProfile = L"hs_6_7";
        break;
    case PipelineStage::TessellationEvaluation:
        targetProfile = L"ds_6_7";
        break;
    case PipelineStage::Geometry:
        targetProfile = L"gs_6_7";
        break;
    case PipelineStage::Fragment:
        targetProfile = L"ps_6_7";
        break;
    case PipelineStage::Compute:
        targetProfile = L"cs_6_7";
        break;
    case PipelineStage::Task:
        targetProfile = L"as_6_7";
        break;
    case PipelineStage::Mesh:
        targetProfile = L"ms_6_7";
        break;
    default:
        throw std::exception("shader stage doesn't supported by dxc");
    }

    std::vector<LPCWSTR> arguments = {
        // (Optional) name of the shader file to be displayed e.g. in an error message
        std::wstring(inputFile.begin(), inputFile.end()).c_str(),
        // ShaderModule main entry point
        L"-E",
        L"main",
        // ShaderModule target profile
        L"-T",
        targetProfile,
        // Compile to SPIRV
        L"-spirv",
    };

    // Compile shader
    DxcBuffer buffer{};
    buffer.Encoding = DXC_CP_ACP;
    buffer.Ptr = blob->GetBufferPointer();
    buffer.Size = blob->GetBufferSize();

    CComPtr<IDxcResult> result{nullptr};
    hres = compiler->Compile(
        &buffer,
        arguments.data(),
        (uint32_t)arguments.size(),
        nullptr,
        IID_PPV_ARGS(&result));

    if (SUCCEEDED(hres))
    {
        result->GetStatus(&hres);
    }

    // Output error if compilation failed
    if (FAILED(hres) && (result))
    {
        CComPtr<IDxcBlobEncoding> errorBlob;
        hres = result->GetErrorBuffer(&errorBlob);
        if (SUCCEEDED(hres) && errorBlob)
        {
            std::cerr << "ShaderModule compilation failed :\n\n"
                      << (const char *)errorBlob->GetBufferPointer();
            throw std::runtime_error("Compilation failed");
        }
    }

    // Get compilation result
    CComPtr<IDxcBlob> code;
    result->GetResult(&code);

    return {};
}

void HLSLCompiler::Destroy()
{
    return;
}