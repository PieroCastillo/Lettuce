//
// Created by piero on 21/09/2024.
//
#include "Lettuce/X3D/Camera3D.hpp"

using namespace Lettuce::X3D;

Camera3D::Camera3D(int width, int height, float angleDegrees, float zNear, float zFar)
{
    projection = glm::perspective(glm::radians(angleDegrees), ((float)width) / ((float)height), zNear, zFar);
}
glm::mat4 Camera3D::GetProjectionView()
{
    return projection * view;
}
void Camera3D::SetPosition(glm::vec3 position)
{
    eye = position;
}
void Camera3D::SetCenter(glm::vec3 center)
{
    this->center = center;
}
void Camera3D::Update()
{
    view = glm::lookAt(eye, center, up);
}