//
// Created by piero on 12/10/2024.
//
#include "Lettuce/X2D/Camera2D.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Lettuce::X2D;

Camera2D::Camera2D(float left, float right, float bottom, float top, float near, float far)
{
    projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
}

void Camera2D::SetPosition(glm::vec3 &position)
{
    viewMatrix = glm::translate(glm::mat4(1.0f), -position);
}

glm::mat4 Camera2D::GetViewProjectionMatrix()
{
    return projectionMatrix * viewMatrix;
}