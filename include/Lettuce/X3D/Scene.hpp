//
// Created by piero on 13/11/2024.
//
#pragma once
#include "common.hpp"

namespace Lettuce::X3D
{
    class Scene
    {
    private:
        // load results
        bool result;
        std::string err;
        std::string warn;
        // main data
        tinygltf::Model model;
        static tinygltf::TinyGLTF loader;
        void check();
        void setup();

    public:
        Scene() {}
        void LoadASCIIFromFile(std::string fileName);
        void LoadASCIIFromString(std::string data, std::string baseDir);
        void LoadBinaryFromFile(const std::string fileName);
        void LoadBinaryFromMemory(unsigned char *data, unsigned int size, std::string baseDir = "");

        void Release();
    };
}