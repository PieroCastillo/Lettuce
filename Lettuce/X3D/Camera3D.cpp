//
// Created by piero on 21/09/2024.
//
#include "Lettuce/X3D/Camera3D.hpp"
#define  GLM_ENABLE_EXPERIMENTAL
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

void Camera3D::MoveByMouse(double dx, double dy, double sensibility)
{
    Rotate(glm::vec3(0,1,0), - dx * sensibility );
    glm::vec3 right = glm::normalize(glm::cross(center - eye, up));
    Rotate(right, - dy * sensibility);
}

void Camera3D::Rotate(glm::vec3 axis, float angle)
{
    glm::vec3 direction = center - eye;
    glm::quat rotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
    direction = rotation * direction;
    up = glm::normalize(rotation* up);
    center = eye + direction;
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