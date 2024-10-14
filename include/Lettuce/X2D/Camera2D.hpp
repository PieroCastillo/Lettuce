//
// Created by piero on 11/10/2024.
//
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lettuce::X2D
{
    class Camera2D
    {
    public:
        Camera2D(float left, float right, float bottom, float top, float near, float far);
        void SetPosition(glm::vec3 &position);
        glm::mat4 GetViewProjectionMatrix();

    private:
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
    };
}