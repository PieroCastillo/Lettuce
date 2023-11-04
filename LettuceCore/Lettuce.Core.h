#include <string>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

namespace LettuceCore {
	class Factory {
	public:
		void Create(std::string appTitle, bool debug = false);
		Platform GetPlatform();
		bool GetDebug();
		std::vector<LettuceCore::GPU> GetGPUs();
		void Destroy();
		VkInstance instance;
	private:
		bool Debug;
		Platform platform;
		std::vector<std::string> extensions;
		std::vector<std::string> enabledExtensions;
		VkDebugUtilsMessengerEXT debugMessenger;
		void checkPlatform();
	};

	struct PhysicalLimits {
		uint32_t maxPushConstantsSize;
	};

	class GPU {
	public:
		void Create(VkPhysicalDevice device);
		std::string GetName();
		GPUType GetType();
		PhysicalLimits GetLimits();
		std::vector<std::string> GetDeviceExtensions();
		VkPhysicalDevice gpu;
	private:
		std::string name;
		GPUType type;
		PhysicalLimits limits;
	};

	class Device {
	public:
		void Create(LettuceCore::Factory factory_, GPU gpu_, void* windowHandlePtr, void* platformHandlePtr);
		void Destroy();
		VkDevice device;
		VkSurfaceKHR surface;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
	private:
		LettuceCore::Factory factory;
		LettuceCore::GPU gpu;
		uint32_t graphicsFamilyIndex;
		uint32_t presentFamilyIndex;
		void* windowHandle;
		void* platformHandle;
		VkExtent2D size;
		void GetIndices();
		void CreateDevice();
		void CreateSurface();
	};

	enum class GPUType { Integrate, Dicrete };
	enum class Platform { Windows, LinuxX11, LinuxWayland, MacOS, iOS };

	void CheckResult(VkResult result, std::string message = "failed to create!") {
		if (result != VK_SUCCESS) {
			throw std::runtime_error(message);
		}
	}

	template <typename T>
	bool contains(std::vector<T> const& vec, T element)
	{
		for (const auto obj : vec) {
			if (obj == element) {
				return true;
			}
		}
		return false;
	}

	uint32_t findQueueFamilyIndex(std::vector<VkQueueFamilyProperties> queueFamilies, VkQueueFlags flags) {
		int i = 0;
		int s = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & flags) {
				s = i;
			}
			i++;
		}
		return i;
	}

	uint32_t findQueuePresentIndex(VkPhysicalDevice gpu, VkSurfaceKHR surface, std::vector<VkQueueFamilyProperties> queueFamilies) {
		int i = 0;
		for (const auto queueFamily : queueFamilies) {
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &presentSupport);

			if (presentSupport) {
				return i;
			}
			i++;
		}
		return -1;
	}
}