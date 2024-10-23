//
// Created by piero on 30/09/2024.
//
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/RenderPass.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D
{
    class RenderContext2D
    {
    private:
        RenderPass renderPass;
        void recontruct();
    public:
        glm::mat4 globalTransform = glm::mat4(1);
        RenderContext2D();
        RenderContext2D(Device device, VkFormat swapchainImageFormat);
        void Record(VkCommandBuffer cmd);
    };
}