//
// Created by piero on 21/09/2024.
//
#include <numbers>
#include <algorithm>
#include "Lettuce/X3D/Camera3D.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace Lettuce::X3D;

Camera3D::Camera3D(int width, int height, float angleDegrees, float zNear, float zFar)
{
    projection = glm::perspective(glm::radians(angleDegrees), ((float)width) / ((float)height), zNear, zFar);
}
glm::mat4 Camera3D::GetProjectionView()
{
    return projection * view;
}

void Camera3D::Move(glm::vec3 dr)
{
    center += dr;
    eye += dr;
}

void Camera3D::Rotate(float dtheta, float dphi)
{
    dphi = std::min(dphi,std::numbers::pi_v<float>);
    glm::vec3 direction = center - eye;
    // first rotate for dtheta
    auto rotMatrix = glm::rotate(glm::mat4(1.0f), dtheta,up);
    direction = (rotMatrix * glm::vec4(direction, 1.0f));
    // next rotate for dphi
    rotMatrix = glm::rotate(glm::mat4(1.0f), dphi, glm::cross(up, direction));
    direction = (rotMatrix * glm::vec4(direction, 1.0f));
    // finally, apply the rotation
    center = eye + direction;
}
void Camera3D::Reset(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    this->eye = eye;
    this->center = center;
    this->up = up;
}
void Camera3D::Update()
{
    view = glm::lookAt(eye, center, up);
}