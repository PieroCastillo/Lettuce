/*
Created by @PieroCastillo on 2026-06-01
*/
#ifndef LETTUCE_UTILS_CAMERA_3D_HPP
#define LETTUCE_UTILS_CAMERA_3D_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Lettuce::Utils
{
    struct Camera3DDesc
    {
        float fovY = 60.0f;
        float aspect = 16.0f / 9.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;

        glm::vec3 position = { 0.0f, 0.0f, 3.0f };
        glm::quat orientation = { 1.0f, 0.0f, 0.0f, 0.0f };
    };

    struct RotateDesc
    {
        float deltaX = 0.0f;
        float deltaY = 0.0f;

        float sensitivity = 0.0025f;
    };

    struct CameraUpdateDesc
    {
        bool keyW = false;
        bool keyA = false;
        bool keyS = false;
        bool keyD = false;

        float deltaTime = 0.0f;
    };

    class Camera3D
    {
    private:
        glm::vec3 m_Position;
        glm::quat m_Orientation;

        float m_FovY;
        float m_Aspect;
        float m_NearPlane;
        float m_FarPlane;
    public:
        explicit Camera3D(const Camera3DDesc& desc)
            : m_Position(desc.position), m_Orientation(desc.orientation), m_FovY(desc.fovY)
            , m_Aspect(desc.aspect), m_NearPlane(desc.nearPlane), m_FarPlane(desc.farPlane)
        {
        }

        void Rotate(const RotateDesc& desc);
        auto Update(const CameraUpdateDesc& desc) -> glm::mat4;

        void SetAspect(float aspect);
    };
};
#endif // LETTUCE_UTILS_CAMERA_3D_HPP