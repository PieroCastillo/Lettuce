//
// created by piero on 25/11/2024
//
#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <exception>
#include "ICompiler.hpp"
#include <dxcapi.h>
#ifdef _WIN32
    #include <atlbase.h>
    #include <windows.h>
    #include <winerror.h>
#endif

namespace Lettuce::Core::Compilers
{
    class HLSLCompiler : public ICompiler
    {
    private:
        HRESULT hres;
        CComPtr<IDxcLibrary> library;
        CComPtr<IDxcCompiler3> compiler;
        CComPtr<IDxcUtils> utils;
    public:
        void Load();
        std::vector<uint32_t> Compile(std::string text, std::string inputFile, PipelineStage stage, bool optimize);
        void Destroy();
    };
}