//
// Created by piero on 30/09/2024.
//
#pragma once
#include <vector>
#include <any>
#include <queue>
#include <memory>
#include <tuple>
#include <type_traits>
#include <concepts>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/Compilers/GLSLCompiler.hpp"
#include "Lettuce/Core/RenderPass.hpp"
#include "Geometries/GeometryBase.hpp"
#include "Lights/LightBase.hpp"
#include "Materials/IMaterial.hpp"
#include "Materials/MaterialBase.hpp"
#include "Effects/EffectBase.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D
{
    template <typename T, typename TData>
    concept InheritsMaterial = std::derived_from<T, Materials::MaterialBase<T, TData>>;

    typedef std::vector<
        std::tuple<
            Geometries::GeometryBase &,
            Materials::IMaterial &,
            std::any &>>
        QueueGeometryMaterial;

    class RenderContext2D final
    {
    private:
        QueueGeometryMaterial materialsPairs;
        uint32_t width, height;
        Device device;
        Compilers::GLSLCompiler compiler;
        Descriptors descriptors;
        void recontruct();
        void renderMaterialPair(VkCommandBuffer cmd, Geometries::GeometryBase &geometry, Materials::IMaterial &material, std::any &data);

    public:
        RenderPass renderPass;
        glm::mat4 globalTransform = glm::mat4(1);
        RenderContext2D() {};
        RenderContext2D(Device &device, VkFormat swapchainImageFormat);
        template <typename T, typename TData>
            requires InheritsMaterial<T, TData>
        void RenderMaterial(Geometries::GeometryBase &geometry, T &material, TData data)
        {
            materialsPairs.push_back(std::make_tuple(geometry, material, data));
        }
        // void AddLight(Geometries::GeometryBase lightArea, Lights::LightBase light);
        // void AddEffect(Geometries::GeometryBase effectArea, Effects::EffectBase effect);
        void Record(VkCommandBuffer cmd, VkImage swapchainImage, uint32_t swapchainImageIndex, VkClearColorValue color);
        void Release();
    };
}