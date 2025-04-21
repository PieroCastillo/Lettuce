//
// Created by piero on 29/12/2024.
//
#pragma once
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/ImageViewResource.hpp"
#include "Lettuce/Core/ResourcePool.hpp"
#include "Lettuce/Core/Device.hpp"

using namespace Lettuce::Core;

namespace Lettuce::Foundation
{
    struct AssetData
    {
        uint32_t width;
        uint32_t height;
        stbi_uc *pixels;
    };

    class AssetLoader
    {
    public:
        std::shared_ptr<Device> device;
        std::shared_ptr<TransferManager> manager;

        AssetLoader(const std::shared_ptr<Device> &device) : device(device)
        {
            manager = std::make_shared<TransferManager>(device);
        }

        using LoadedImages = std::tuple<std::shared_ptr<ResourcePool>,
                                        std::vector<std::shared_ptr<ImageResource>>,
                                        std::vector<std::shared_ptr<ImageViewResource>>>;

        LoadedImages LoadImagesToDeviceMemory(std::shared_ptr<Device> device, std::vector<std::string> assetsFilenames)
        {
            auto pool_temp = std::make_shared<ResourcePool>();
            auto pool_images = std::make_shared<ResourcePool>();

            std::vector<std::shared_ptr<ImageResource>> images_bindable;
            std::vector<std::shared_ptr<ImageViewResource>> imgVw_bindable;

            std::vector<std::shared_ptr<BufferResource>> imgs_temp;
            std::vector<std::pair<void *, uint32_t>> imgPtrs;

            for (auto &assetFilename : assetsFilenames)
            {
                int texWidth, texHeight, texChannels;
                if (!std::filesystem::exists(assetFilename))
                {
                    std::cout << " file " << assetFilename << " does not exist" << std::endl;
                    std::abort();
                }
                stbi_uc *pixels = stbi_load(assetFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

                if (!pixels)
                {
                    std::abort();
                }

                imgPtrs.push_back({(void *)pixels, 4 * texHeight * texWidth});

                auto img_temp = std::make_shared<BufferResource>(device, 4 * texHeight * texWidth, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);

                auto img_dst = std::make_shared<ImageResource>(device,
                                                               (uint32_t)texWidth,
                                                               (uint32_t)texHeight,
                                                               1, VkImageType::VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                               1, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_PREINITIALIZED);

                imgs_temp.push_back(img_temp);
                images_bindable.push_back(img_dst);

                pool_temp->AddResource(img_temp);
                pool_images->AddResource(img_dst);
            }

            pool_temp->Bind(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            pool_images->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            auto [dataPtr, sz] = AllocAllInOne(imgPtrs);
            pool_temp->Map(0, pool_temp->GetSize());
            pool_temp->SetData(dataPtr, 0, sz);
            pool_temp->UnMap();

            manager->Prepare();
            for (int i = 0; i < imgs_temp.size(); i++)
            {
                manager->AddTransference(imgs_temp[i], images_bindable[i], 0, TransferType::HostToDevice);
            }
            manager->TransferAll();

            for (auto &img : images_bindable)
            {
                auto vw = std::make_shared<ImageViewResource>(device, img);
                imgVw_bindable.push_back(vw);
            }

            free(dataPtr);

            for (auto &[ptr, _] : imgPtrs)
            {
                free(ptr);
            }

            for (auto &img : imgs_temp)
            {
                img->Release();
            }

            pool_temp->Release();

            return {pool_images, images_bindable, imgVw_bindable};
        }

        using LoadedImageArray = std::tuple<std::shared_ptr<ResourcePool>,
                                            std::shared_ptr<ImageResource>,
                                            std::shared_ptr<ImageViewResource>>;

        LoadedImageArray LoadImageArrayToDeviceMemory(std::shared_ptr<Device> device, std::vector<std::string> assetsFilenames)
        {
            auto pool_temp = std::make_shared<ResourcePool>();
            auto pool_images = std::make_shared<ResourcePool>();

            std::shared_ptr<ImageResource> images_bindable;
            std::shared_ptr<ImageViewResource> imgVw_bindable;

            std::vector<std::shared_ptr<BufferResource>> imgs_temp;
            std::vector<std::pair<void *, uint32_t>> imgPtrs;

            for (auto &assetFilename : assetsFilenames)
            {
                int texWidth, texHeight, texChannels;
                if (!std::filesystem::exists(assetFilename))
                {
                    std::cout << " file " << assetFilename << " does not exist" << std::endl;
                    std::abort();
                }
                stbi_uc *pixels = stbi_load(assetFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

                if (!pixels)
                {
                    std::abort();
                }

                imgPtrs.push_back({(void *)pixels, 4 * texHeight * texWidth});

                auto img_temp = std::make_shared<BufferResource>(device, 4 * texHeight * texWidth, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT);

                // auto img_dst = std::make_shared<ImageResource>(device,
                //                                                (uint32_t)texWidth,
                //                                                (uint32_t)texHeight,
                //                                                1, VkImageType::VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                //                                                1, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_PREINITIALIZED);

                imgs_temp.push_back(img_temp);
                // images_bindable.push_back(img_dst);

                pool_temp->AddResource(img_temp);
                // pool_images->AddResource(img_dst);
            }

            pool_temp->Bind(device, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            pool_images->Bind(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            auto [dataPtr, sz] = AllocAllInOne(imgPtrs);
            pool_temp->Map(0, pool_temp->GetSize());
            pool_temp->SetData(dataPtr, 0, sz);
            pool_temp->UnMap();

            manager->Prepare();
            for (int i = 0; i < imgs_temp.size(); i++)
            {
                manager->AddTransference(imgs_temp[i], images_bindable[i], 0, TransferType::HostToDevice);
            }
            manager->TransferAll();

            for (auto &img : images_bindable)
            {
                auto vw = std::make_shared<ImageViewResource>(device, img);
                imgVw_bindable.push_back(vw);
            }

            free(dataPtr);

            for (auto &[ptr, _] : imgPtrs)
            {
                free(ptr);
            }

            for (auto &img : imgs_temp)
            {
                img->Release();
            }

            pool_temp->Release();

            return {pool_images, images_bindable, imgVw_bindable};
        }

        void Release()
        {
            manager->Release();
        }
    };
}