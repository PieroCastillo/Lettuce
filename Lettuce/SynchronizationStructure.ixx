//
// Created by piero on 14/03/2024.
//
module;

#include <iostream>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>

export module Lettuce:SynchronizationStructure;

import :Device;
import :Utils;

export namespace Lettuce::Core
{
    class SynchronizationStructure
    {
    public:
        Device _device;
        std::vector<VkFence> fences;
        std::vector<VkSemaphore> semaphores;

        void Create(Device &device, uint32_t fencesCount, uint32_t semaphoresCount) {
            _device = device;
            fences.resize(fencesCount);
            semaphores.resize(semaphoresCount);

            VkFenceCreateInfo fenceCI ={
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
            };
            VkSemaphoreCreateInfo semaphoreCI = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
            for(int i = 0; i < fencesCount; i++) {
                checkResult(vkCreateFence(_device._device, &fenceCI, nullptr, &fences[i]), "fence #" + std::to_string(i) + " created succesfully");
            }
            for(int i = 0; i < semaphoresCount; i++) {
                checkResult(vkCreateSemaphore(_device._device, &semaphoreCI, nullptr, &semaphores[i]), "semaphore #" + std::to_string(i) + " created succesfully");
            }
        }

        void WaitForFence(uint32_t index) {
            vkWaitForFences(_device._device, 1, &fences[index], VK_TRUE, std::numeric_limits<uint32_t>::max());
        }

        void WaitForAllFences() {
            vkWaitForFences(_device._device, fences.size(), fences.data(), VK_TRUE, std::numeric_limits<uint32_t>::max());
        }

        void ResetFence(uint32_t index) {
            vkResetFences(_device._device, 1, &fences[index]);
        }

        void ResetAllFences() {
            vkResetFences(_device._device, fences.size(), fences.data());
        }

        void Destroy() {
            for(int i = 0; i < fences.size(); i++) {
                vkDestroyFence(_device._device, fences[i], nullptr);
            }
            for(int i = 0; i < semaphores.size() ; i++) {
                vkDestroySemaphore(_device._device, semaphores[i], nullptr);
            }
        }
    };
}