#include <iostream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "Lettuce.Core.h"

//#define VK_USE_PLATFORM_WIN32_KHR
//#define VK_USE_PLATFORM_ANDROID_KHR
//#define VK_USE_PLATFORM_WAYLAND_KHR
///#define VK_USE_PLATFORM_XCB_KHR
//#define VK_USE_PLATFORM_IOS_MVK
//#define VK_USE_PLATFORM_MACOS_MVK


// void LettuceCore::Device::Create(LettuceCore::Factory factory_, GPU gpu_, void* windowHandlePtr, void* platformHandlePtr)
// {
// 	factory = factory_;
// 	gpu = gpu_;
// 	windowHandle = windowHandlePtr;
// 	platformHandle = platformHandlePtr;
// 	CreateSurface();
// 	GetIndices();
// 	CreateDevice();
// 	vkGetDeviceQueue(device, graphicsFamilyIndex, 0, &graphicsQueue);
// 	vkGetDeviceQueue(device, presentFamilyIndex, 0, &presentQueue);
// }
// void LettuceCore::Device::GetIndices() 
// {
// 	uint32_t queueFamilyCount = 0;
// 	vkGetPhysicalDeviceQueueFamilyProperties(gpu.gpu, &queueFamilyCount, nullptr);

// 	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
// 	vkGetPhysicalDeviceQueueFamilyProperties(gpu.gpu, &queueFamilyCount, queueFamilies.data());

// 	graphicsFamilyIndex = findQueueFamilyIndex(queueFamilies, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
// 	presentFamilyIndex = findQueuePresentIndex(gpu.gpu, surface, queueFamilies);
// 	if (presentFamilyIndex == -1) {
// 		std::runtime_error("doesn't present support for surface presentation");
// 	}
// }
// void LettuceCore::Device::CreateDevice()
// {
// 	float queuePriority = 1.0f;
// 	std::vector<VkDeviceQueueCreateInfo> queuesCI;
// 	std::vector<uint32_t> uniqueQueueFamilies = { graphicsFamilyIndex, presentFamilyIndex };

// 	for (const auto index : uniqueQueueFamilies) {
// 		VkDeviceQueueCreateInfo queueCI;
// 		queueCI.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
// 		queueCI.queueFamilyIndex = graphicsFamilyIndex;
// 		queueCI.queueCount = 1;
// 		queueCI.pQueuePriorities = &queuePriority;
		
// 		queuesCI.push_back(queueCI);
// 	}

// 	VkPhysicalDeviceFeatures features;
// 	//vkGetPhysicalDeviceFeatures(gpu.gpu, &features);

// 	VkDeviceCreateInfo deviceCI;
// 	deviceCI.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
// 	deviceCI.queueCreateInfoCount = 1;
// 	deviceCI.pQueueCreateInfos = queuesCI.data();
// 	deviceCI.enabledLayerCount = queuesCI.size();
// 	deviceCI.ppEnabledExtensionNames = nullptr;
// 	deviceCI.pEnabledFeatures = &features;
	
// 	CheckResult(vkCreateDevice(this->gpu.gpu, &deviceCI, nullptr, &device));
// }

// //TODO: impl surface for all platforms
// void LettuceCore::Device::CreateSurface()
// {
// #if defined(VK_USE_PLATFORM_WIN32_KHR)
// 	VkWin32SurfaceCreateInfoKHR win32surfaceCI;
// 	win32surfaceCI.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
// 	win32surfaceCI.hinstance = (HINSTANCE)(this->platformHandle);
// 	win32surfaceCI.hwnd = (HWND)(this->windowHandle);
	
// 	CheckResult(vkCreateWin32SurfaceKHR(factory.instance, &win32surfaceCI, nullptr, &surface));
// #elif defined(VK_USE_PLATFORM_ANDROID_KHR)
// 	VkAndroidSurfaceCreateInfoKHR androidCI;
// 	androidCI.sType = VkStructureType::VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
// 	androidCI.window = (ANativeWindow*)(this->windowHandle;
	
// 	CheckResult(vkCreateAndroidSurfaceKHR(factory.instance, &androidCI, nullptr, &surface));
// #elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
// 	VkWaylandSurfaceCreateInfoKHR waylandCI;
// 	waylandCI.sType = VkStructureType::VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
// 	waylandCI.display = (wl_display*)(this->platformHandle);
// 	waylandCI.surface = (wl_surface*)(this->windowHandle);
// 	};
// #elif defined(VK_USE_PLATFORM_XCB_KHR)
// #elif defined(VK_USE_PLATFORM_IOS_MVK)
// #elif defined(VK_USE_PLATFORM_MACOS_MVK)
// #endif
// }

// void LettuceCore::Device::Destroy()
// {
// 	vkDestroyDevice(device, nullptr);
// 	vkDestroySurfaceKHR(factory.instance, surface, nullptr);
// }