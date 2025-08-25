/*
Created by @PieroCastillo on 2025-07-25
*/
#ifndef LETTUCE_CORE_TEXTURE_VIEW_HPP
#define LETTUCE_CORE_TEXTURE_VIEW_HPP

// standard headers
#include <memory>

// project headers
#include "Common.hpp"

namespace Lettuce::Core
{
    struct TextureViewCreateInfo
    {
        uint32_t fromLevel, toLevel;
        uint32_t fromLayer, toLayer;
        bool asArray;
        bool asCube;
        std::weak_ptr<TextureArray> texture;
    };

    /*
        Image View Type	              Compatible Image Types
        VK_IMAGE_VIEW_TYPE_1D         VK_IMAGE_TYPE_1D
        VK_IMAGE_VIEW_TYPE_1D_ARRAY   VK_IMAGE_TYPE_1D
        VK_IMAGE_VIEW_TYPE_2D         VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D
        VK_IMAGE_VIEW_TYPE_2D_ARRAY   VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D
        VK_IMAGE_VIEW_TYPE_CUBE       VK_IMAGE_TYPE_2D
        VK_IMAGE_VIEW_TYPE_CUBE_ARRAY VK_IMAGE_TYPE_2D
        VK_IMAGE_VIEW_TYPE_3D         VK_IMAGE_TYPE_3D
        	              
        Compatible Image Types      Image View Type
        VK_IMAGE_TYPE_1D            VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_1D_ARRAY
        VK_IMAGE_TYPE_2D            VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
        VK_IMAGE_TYPE_3D            VK_IMAGE_VIEW_TYPE_3D
    */

    class TextureView
    {
    private:

    public:
        VkDevice m_device;
        VkImage m_image;
        VkImageView m_imageView;

        LettuceResult Create(const std::weak_ptr<IDevice>& device, const TextureViewCreateInfo& createInfo);
        void Release();
    };
}
#endif // LETTUCE_CORE_TEXTURE_VIEW_HPP