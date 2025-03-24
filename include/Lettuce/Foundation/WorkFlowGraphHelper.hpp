//
// Created by piero on 22/03/2025.
//
#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <algorithm>
#include <memory>
#include <thread>
#include <vector>
#include <variant>
#include "Lettuce/Core/BufferResource.hpp"
#include "Lettuce/Core/ImageResource.hpp"
#include "Lettuce/Core/Device.hpp"
#include "Lettuce/Core/Semaphore.hpp"

#include "Lettuce/Core/GraphicsPipeline.hpp"
#include "Lettuce/Core/ComputePipeline.hpp"
#include "Lettuce/Core/PipelineLayout.hpp"
#include "Lettuce/Core/Descriptors.hpp"

#include "WorkFlowGraph.hpp"

using namespace Lettuce::Core;
using namespace Lettuce::Foundation;

namespace Lettuce::Foundation
{
    // // buffer
    // BufferAsResourceInfo AsStorage(const std::shared_ptr<BufferResource> &buffer,ResourceUsage usage)
    // {
    //     BufferAsResourceInfo info = {
    //         .bufferPtr = buffer,
    //         // .stageBit = 
    //         .usage = usage,
    //     };
    //     return info;
    // }
    // BufferAsResourceInfo AsVertexBuffer(const std::shared_ptr<BufferResource> &buffer,ResourceUsage usage)
    // {
    //     BufferAsResourceInfo info = {
    //         .bufferPtr = buffer,
    //         .stageBit = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
    //         .usage = usage,
    //     };
    //     return info;
    // }
    // BufferAsResourceInfo AsIndexBuffer(const std::shared_ptr<BufferResource> &buffer,ResourceUsage usage)
    // {
    //     BufferAsResourceInfo info = {
    //         .bufferPtr = buffer,
    //         .stageBit = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
    //         .usage = usage,

    //     };
    //     return info;
    // }
    // // images
    // ImageAsResourceInfo AsTexture(const std::shared_ptr<ImageResource> &image,ResourceUsage usage)
    // {
    //     ImageAsResourceInfo info = {
    //         .img = image,
    //         .stageBit = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
    //         .usage = usage,
    //     };
    //     return info;
    // }

    // // image views
    // ImageViewAsResourceInfo AsColorAttachment(const std::shared_ptr<ImageViewResource> &imageView,ResourceUsage usage)
    // {
    //     ImageViewAsResourceInfo info = {
    //         .imgVwPtr = imageView,
    //         .stageBit = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
    //         .usage = usage,
            
    //     };
    //     return info;
    // }
    // ImageViewAsResourceInfo AsDepthAttachment(const std::shared_ptr<ImageViewResource> &imageView,ResourceUsage usage)
    // {
    //     ImageViewAsResourceInfo info = {
    //         .imgVwPtr = imageView,
    //         .stageBit = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,

    //         .usage = usage,
    //     };
    //     return info;
    // }
    // ImageViewAsResourceInfo AsInputAttachment(const std::shared_ptr<ImageViewResource> &imageView,ResourceUsage usage)
    // {
    //     ImageViewAsResourceInfo info = {
    //         .imgVwPtr = imageView,

    //         .stageBit = VK_PIPELINE_STAGE_2_,
    //         .usage = usage,
    //     };
    //     return info;
    // }
}