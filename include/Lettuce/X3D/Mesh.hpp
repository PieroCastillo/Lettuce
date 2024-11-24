//
// Created by piero on 13/11/2024.
//
#pragma once
#include "common.hpp"

namespace Lettuce::X3D
{
    class Mesh
    {
    private:
        // main data
        void setup();

    public:
        Mesh() {}

        void Release();
    };
}