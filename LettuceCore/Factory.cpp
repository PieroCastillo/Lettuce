#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include "Lettuce.Core.h"

#define uint uint32_t

// void LettuceCore::Factory::Create(std::string apptitle, bool debugMode = false) {
// 	VkApplicationInfo appInfo;
// 	appInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
// 	appInfo.pApplicationName = apptitle.c_str();
// 	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
// 	appInfo.pEngineName = "Lettuce Render Engine";
// 	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
// 	appInfo.apiVersion = VK_API_VERSION_1_0;

// 	VkInstanceCreateInfo ci;
// 	ci.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
// 	ci.pApplicationInfo = &appInfo;

// 	uint extensionCount;
// 	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
// 	std::vector<VkExtensionProperties> tempExtensions(extensionCount);
// 	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, tempExtensions.data());
// 	for (const auto ext : tempExtensions) {
// 		extensions.push_back(std::string(ext.extensionName));
// 	}

// 	checkPlatform();

// 	ci.enabledLayerCount = 0;
// 	ci.ppEnabledExtensionNames = nullptr;

// 	const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
// 	VkDebugUtilsMessengerCreateInfoEXT duCI;
// 	if (debugMode)
// 	{
// 		uint instanceLayerCount;
// 		bool validationLayerPresent = false;
// 		bool debugUtilsExtensionPresent = contains<std::string>(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

// 		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
// 		std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
// 		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());

// 		for (VkLayerProperties layer : instanceLayerProperties) {
// 			if (strcmp(layer.layerName, validationLayerName) == 0) {
// 				validationLayerPresent = true;
// 				break;
// 			}
// 		}

// 		if (validationLayerPresent && debugUtilsExtensionPresent) {
// 			this->Debug = true;
// 			enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

// 			ci.ppEnabledLayerNames = &validationLayerName;
// 			ci.enabledLayerCount = 1;

// 			duCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
// 			duCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
// 			duCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
// 			duCI.pfnUserCallback = debugCallback;
// 			duCI.pUserData = nullptr;

// 			ci.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&duCI;
// 		}
// 		else {
// 			this->Debug = false;
// 			std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
// 		}
// 	}

// 	ci.enabledExtensionCount = enabledExtensions.size();
// 	ci.ppEnabledExtensionNames = (char**)enabledExtensions.data();

// 	CheckResult(vkCreateInstance(&ci, nullptr, &(Factory::instance)));

// 	if (this->Debug) {
// 		duCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
// 		duCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
// 		duCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
// 		duCI.pfnUserCallback = debugCallback;
// 		duCI.pUserData = nullptr;

// 		if (CreateDebugUtilsMessengerEXT(instance, &duCI, nullptr, &debugMessenger) != VK_SUCCESS) {
// 			throw std::runtime_error("failed to set up debug messenger!");
// 		}
// 	}
// }


// void LettuceCore::Factory::Destroy() {
// 	if (this->Debug)
// 	{
// 		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
// 	}
// 	vkDestroyInstance(instance, nullptr);
// }

// void LettuceCore::Factory::checkPlatform()
// {
// 	if (contains<std::string>(extensions, VK_KHR_SURFACE_EXTENSION_NAME)) {
// 		enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
// 	}

// #if defined(VK_USE_PLATFORM_WIN32_KHR)
// 	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
// 	platform = Platform::Windows;
// #elif defined(VK_USE_PLATFORM_ANDROID_KHR)
// 	enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
// 	platform = Platform::Android;
// #elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
// 	enabledExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
// 	platform = Platform::LinuxWayland;
// #elif defined(VK_USE_PLATFORM_XCB_KHR)
// 	enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
// 	platform = Platform::LinuxX11;
// #elif defined(VK_USE_PLATFORM_IOS_MVK)
// 	enabledExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
// 	platform = Platform::iOS;
// #elif defined(VK_USE_PLATFORM_MACOS_MVK)
// 	instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
// 	platform = Platform::MacOS;
// #endif
// }

// LettuceCore::Platform LettuceCore::Factory::GetPlatform()
// {
// 	return this->platform;
// }

// bool LettuceCore::Factory::GetDebug()
// {
// 	return this->Debug;
// }

// std::vector<LettuceCore::GPU> LettuceCore::Factory::GetGPUs() {
// 	std::vector<LettuceCore::GPU> gpus;

// 	uint gpuCount;
// 	vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
// 	std::vector<VkPhysicalDevice> devices(gpuCount);
// 	vkEnumeratePhysicalDevices(instance, &gpuCount, devices.data());

// 	for (const auto device : devices) {
// 		LettuceCore::GPU gpu;
// 		gpu.Create(device);
// 		gpus.push_back(gpu);
// 	}
// 	return gpus;
// }

// static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
// 	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
// 	VkDebugUtilsMessageTypeFlagsEXT messageType,
// 	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
// 	void* pUserData) {
// 	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
// 		// Message is important enough to show
// 		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
// 	}

// 	return VK_FALSE;
// }

// VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
// 	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
// 	if (func != nullptr) {
// 		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
// 	}
// 	else {
// 		return VK_ERROR_EXTENSION_NOT_PRESENT;
// 	}
// }

// void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
// 	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
// 	if (func != nullptr) {
// 		func(instance, debugMessenger, pAllocator);
// 	}
// }