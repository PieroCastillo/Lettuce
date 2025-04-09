//
// Created by piero on 7/04/2025.
//
#include <string>
#include <optional>

namespace Lettuce::Geometry::Materials
{
    class MaterialFamily
    {
        std::string materialFamilyName;
        uint32_t materialFamilyID;
        std::string vertexCode;
        std::string tessellationEvaluationCode;
        std::string tessellationControlCode;
        std::string geometryCode;

    };
}