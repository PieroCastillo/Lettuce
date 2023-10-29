#include <iostream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "Lettuce.Core.h"

// void LettuceCore::GPU::Create(VkPhysicalDevice device) {
// 	gpu = device;
// 	VkPhysicalDeviceProperties deviceProps;
// 	vkGetPhysicalDeviceProperties(gpu, &deviceProps);

// 	if (deviceProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
// 		type = GPUType::Dicrete;
// 	}
// 	else if (deviceProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
// 		type = GPUType::Integrate;
// 	}

// 	name = deviceProps.deviceName;

// 	limits.maxPushConstantsSize = deviceProps.limits.maxPushConstantsSize;
// }

// std::vector<std::string> LettuceCore::GPU::GetDeviceExtensions()
// {
// 	uint32_t extCount;
// 	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extCount, nullptr);
// 	std::vector<VkExtensionProperties> tempExtensions(extCount);
// 	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extCount, tempExtensions.data());

// 	std::vector<std::string> extensions(extCount);
// 	for (const auto ext : tempExtensions) {
// 		extensions.push_back(ext.extensionName);
// 	}

// 	return extensions;
// }


// std::string LettuceCore::GPU::GetName() {
// 	return name;
// }

// LettuceCore::GPUType LettuceCore::GPU::GetType()
// {
// 	return type;
// }

// LettuceCore::PhysicalLimits LettuceCore::GPU::GetLimits()
// {
// 	return limits;
// }