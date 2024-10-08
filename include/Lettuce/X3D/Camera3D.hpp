//
// Created by piero on 21/09/2024.
//
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lettuce::X3D
{
    /// a 3D Camera
    /// theta:
    class Camera3D
    {
    private:
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec3 eye, center;
        glm::vec3 up = glm::vec3(1.0f, 1.0f, 0.0f);

    public:
        Camera3D() {}
        Camera3D(int width, int height, float angleDegrees = 45.0f, float zNear = 0.1f, float zFar = 100.0f);
        glm::mat4 GetProjectionView();
        void SetPosition(glm::vec3 position);
        void SetCenter(glm::vec3 center);
        void Update();
    };
}