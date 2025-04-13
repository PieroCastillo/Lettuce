//
// Created by piero on 13/11/2024.
//
#pragma once

#include <vector>

#include "commonX3D.hpp"

namespace Lettuce::X3D
{
    struct MeshInfo
    {
        uint32_t vertexBufferSize;
        uint32_t vertexBufferOffset;       
        uint32_t indexBufferSize;
        uint32_t indexBufferOffset;
    };

    struct Mesh
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec4> tangents;
        std::vector<uint32_t> indices;
    };
}