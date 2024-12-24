//
// Created by piero on 21/09/2024.
//
#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace Lettuce::X3D
{
    /// a 3D Camera
    /// theta:
    class Camera3D
    {
    private:
        glm::mat4 projection;
        glm::mat4 view;

    public:
        glm::vec3 eye, center;
        glm::vec3 up;
        Camera3D() {}
        Camera3D(int width, int height, float angleDegrees = 45.0f, float zNear = 0.1f, float zFar = 100.0f);
        glm::mat4 GetProjectionView();
        void Rotate(float dtheta, float dphi);
        void Move(glm::vec3 dr);
        void Reset(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
        void Update();
    };
}