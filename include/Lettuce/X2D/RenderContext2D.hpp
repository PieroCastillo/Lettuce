//
// Created by piero on 30/09/2024.
//
#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lettuce/Core/RenderPass.hpp"
#include "Geometries/GeometryBase.hpp"
#include "Lights/LightBase.hpp"
#include "Materials/MaterialBase.hpp"
#include "Effects/EffectBase.hpp"

using namespace Lettuce::Core;

namespace Lettuce::X2D
{
    class RenderContext2D
    {
    private:
        std::vector<std::pair<std::shared_ptr<Geometries::GeometryBase>, std::shared_ptr<Materials::MaterialBase>>> materialsPairs;
        uint32_t width, height;
        RenderPass renderPass;
        void recontruct();
        void renderMaterialPair(VkCommandBuffer cmd, std::shared_ptr<Geometries::GeometryBase> geometryPtr, std::shared_ptr<Materials::MaterialBase> materialPtr);

    public:
        glm::mat4 globalTransform = glm::mat4(1);
        RenderContext2D();
        RenderContext2D(Device device, VkFormat swapchainImageFormat);
        void RenderMaterial(Geometries::GeometryBase geometry, Materials::MaterialBase material);
        // void AddLight(Geometries::GeometryBase lightArea, Lights::LightBase light);
        // void AddEffect(Geometries::GeometryBase effectArea, Effects::EffectBase effect);
        void Record(VkCommandBuffer cmd, VkImageView swapchainImage);
        void Release();
    };
}