//
// Created by piero on 30/09/2024.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lettuce::X2D
{
    class RenderContext
    {
    public:
        glm::mat4 globalTransform = glm::mat4(1);
        RenderContext();
        void Record(VkCommandBuffer cmd);
    };
}