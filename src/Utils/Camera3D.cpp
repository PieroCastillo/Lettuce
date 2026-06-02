#include "Lettuce/Utils/Camera3D.hpp"

using namespace Lettuce::Utils;

void Camera3D::Rotate(const RotateDesc& desc)
{
    auto yaw = glm::angleAxis(-desc.deltaX * desc.sensitivity, glm::vec3(0.0f, 1.0f, 0.0f));
    auto right = m_Orientation * glm::vec3(1.0f, 0.0f, 0.0f);
    auto pitch = glm::angleAxis(-desc.deltaY * desc.sensitivity, right);

    m_Orientation = glm::normalize(pitch * yaw * m_Orientation);
}

auto Camera3D::Update(const CameraUpdateDesc& desc) -> glm::mat4
{
    float speed = 5.0f * desc.deltaTime;

    auto forward = m_Orientation * glm::vec3(0.0f, 0.0f, -1.0f);
    auto right = m_Orientation * glm::vec3(1.0f, 0.0f, 0.0f);

    if (desc.keyW) m_Position += forward * speed;
    if (desc.keyS) m_Position -= forward * speed;
    if (desc.keyA) m_Position -= right * speed;
    if (desc.keyD) m_Position += right * speed;

    auto rotation = glm::toMat4(glm::conjugate(m_Orientation));
    auto translation = glm::translate(glm::mat4(1.0f), -m_Position);
    auto view = rotation * translation;
    auto projection = glm::perspective(glm::radians(m_FovY), m_Aspect, m_NearPlane, m_FarPlane);

    projection[1][1] *= -1.0f;

    return projection * view;
}

void Camera3D::SetAspect(float aspect)
{
    m_Aspect = aspect;
}