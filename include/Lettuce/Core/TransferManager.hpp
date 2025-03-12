//
// Created by piero on 19/02/2025.
//
#pragma once
#include <memory>
#include "Device.hpp"
#include "IReleasable.hpp"
#include "BufferResource.hpp"
#include "ImageResource.hpp"
#include "Semaphore.hpp"
#include "ResourcePool.hpp"

namespace Lettuce::Core
{
    enum class TransferType
    {
        HostToDevice,
        DeviceToHost,
        DeviceToDevice,
    };

    /// @brief TransferManager is the class dedicated to transfer buffers/images between host and device memory.
    ///
    /// The transferences from host to host memory are done immediatly.
    ///
    /// First, call Prepare(), to set up the Command Buffer.
    /// the HostToDevice, DeviceToDevice and DeviceToHost transferences
    /// are recorded into a Command Buffer using AddTransfer()
    ///
    /// To execute the transferences use TransferAll()
    class TransferManager : public IReleasable
    {
    public:
        std::shared_ptr<Device> _device;
        std::shared_ptr<Semaphore> transferFinished;
        uint64_t transferFinishedValue = 0;
        VkCommandPool cmdPool;
        VkCommandBuffer cmd;
        uint32_t _queueFamily = 0;

        TransferManager(const std::shared_ptr<Device> &device);
        void Release();

        void Prepare();
        void AddTransference(const std::shared_ptr<BufferResource> &src, const std::shared_ptr<ImageResource> &dst, uint32_t mipLevel, TransferType type);
        void AddTransference(const std::shared_ptr<BufferResource> &src, const std::shared_ptr<BufferResource> &dst, TransferType type);
        void AddTransference(VkImageSubresourceLayers srcSubresource, VkImageSubresourceLayers dstSubresource, const std::shared_ptr<ImageResource> &src, const std::shared_ptr<ImageResource> &dst, TransferType type);
        void TransferAll();
    };
}