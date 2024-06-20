// //
// // Created by piero on 17/02/2024.
// //

// module;
// #include <mutex>
// #include <vulkan/vulkan.h>
// #if defined(_WIN32)
// #include <fcntl.h>
// #ifndef NOMINMAX
// #define NOMINMAX
// #endif
// #include <windows.h>
// #endif // _WIN32

// #if defined(__linux__) || defined(__APPLE__)
// #include <dlfcn.h>
// #endif

// export module Lettuce:Loader;

// export namespace Lettuce::Core
// {

//     class VulkanFunctions
//     {
//         std::mutex init_mutex;

//     public:
// #if defined(__linux__) || defined(__APPLE__)
//         void *library = nullptr;
// #elif defined(_WIN32)
//         HMODULE library = nullptr;
// #endif

//         bool load_vulkan_library()
//         {
//             // Can immediately return if it has already been loaded
//             if (library)
//             {
//                 return true;
//             }
// #if defined(__linux__)
//             library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
//             if (!library)
//                 library = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
// #elif defined(__APPLE__)
//             library = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
//             if (!library)
//                 library = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
//             if (!library)
//                 library = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
// #elif defined(_WIN32)
//             library = LoadLibrary(TEXT("vulkan-1.dll"));
// #else
//             assert(false && "Unsupported platform");
// #endif
//             if (!library)
//                 return false;
//             load_func(ptr_vkGetInstanceProcAddr, "vkGetInstanceProcAddr");
//             return ptr_vkGetInstanceProcAddr != nullptr;
//         }

//         template <typename T>
//         void load_func(T &func_dest, const char *func_name)
//         {
// #if defined(__linux__) || defined(__APPLE__)
//             func_dest = reinterpret_cast<T>(dlsym(library, func_name));
// #elif defined(_WIN32)
//             func_dest = reinterpret_cast<T>(GetProcAddress(library, func_name));
// #endif
//         }
//         void close()
//         {
// #if defined(__linux__) || defined(__APPLE__)
//             dlclose(library);
// #elif defined(_WIN32)
//             FreeLibrary(library);
// #endif
//             library = 0;
//         }

//     public:
//         bool init_vulkan_funcs(PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr = nullptr)
//         {
//             std::lock_guard<std::mutex> lg(init_mutex);
//             if (fp_vkGetInstanceProcAddr != nullptr)
//             {
//                 ptr_vkGetInstanceProcAddr = fp_vkGetInstanceProcAddr;
//             }
//             else
//             {
//                 bool ret = load_vulkan_library();
//                 if (!ret)
//                     return false;
//             }

//             fp_vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
//                 ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties"));
//             fp_vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
//                 ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties"));
//             fp_vkEnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
//                 ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
//             fp_vkCreateInstance =
//                 reinterpret_cast<PFN_vkCreateInstance>(ptr_vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));
//             return true;
//         }

//     public:
//         template <typename T>
//         void get_inst_proc_addr(T &out_ptr, const char *func_name)
//         {
//             out_ptr = reinterpret_cast<T>(ptr_vkGetInstanceProcAddr(instance, func_name));
//         }

//         template <typename T>
//         void get_device_proc_addr(VkDevice device, T &out_ptr, const char *func_name)
//         {
//             out_ptr = reinterpret_cast<T>(fp_vkGetDeviceProcAddr(device, func_name));
//         }

//         PFN_vkGetInstanceProcAddr ptr_vkGetInstanceProcAddr = nullptr;
//         VkInstance instance = nullptr;

//         PFN_vkEnumerateInstanceExtensionProperties fp_vkEnumerateInstanceExtensionProperties = nullptr;
//         PFN_vkEnumerateInstanceLayerProperties fp_vkEnumerateInstanceLayerProperties = nullptr;
//         PFN_vkEnumerateInstanceVersion fp_vkEnumerateInstanceVersion = nullptr;
//         PFN_vkCreateInstance fp_vkCreateInstance = nullptr;

//         PFN_vkDestroyInstance fp_vkDestroyInstance = nullptr;
//         PFN_vkCreateDebugUtilsMessengerEXT fp_vkCreateDebugUtilsMessengerEXT = nullptr;
//         PFN_vkDestroyDebugUtilsMessengerEXT fp_vkDestroyDebugUtilsMessengerEXT = nullptr;
//         PFN_vkEnumeratePhysicalDevices fp_vkEnumeratePhysicalDevices = nullptr;
//         PFN_vkGetPhysicalDeviceFeatures fp_vkGetPhysicalDeviceFeatures = nullptr;
//         PFN_vkGetPhysicalDeviceFeatures2 fp_vkGetPhysicalDeviceFeatures2 = nullptr;
//         PFN_vkGetPhysicalDeviceFeatures2KHR fp_vkGetPhysicalDeviceFeatures2KHR = nullptr;
//         PFN_vkGetPhysicalDeviceProperties fp_vkGetPhysicalDeviceProperties = nullptr;
//         PFN_vkGetPhysicalDeviceQueueFamilyProperties fp_vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
//         PFN_vkGetPhysicalDeviceMemoryProperties fp_vkGetPhysicalDeviceMemoryProperties = nullptr;
//         PFN_vkEnumerateDeviceExtensionProperties fp_vkEnumerateDeviceExtensionProperties = nullptr;
//         PFN_vkEnumerateDeviceLayerProperties fp_vkEnumerateDeviceLayerProperties = nullptr;

//         PFN_vkCreateDevice fp_vkCreateDevice = nullptr;
//         PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr = nullptr;

//         PFN_vkDestroySurfaceKHR fp_vkDestroySurfaceKHR = nullptr;
//         PFN_vkGetPhysicalDeviceSurfaceSupportKHR fp_vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
//         PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fp_vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
//         PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fp_vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
//         PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fp_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;

//         PFN_vkCmdBeginRendering fp_vkCmdBeginRendering = nullptr;
//         PFN_vkCmdEndRendering fp_vkCmdEndRendering = nullptr;
//         PFN_vkQueueSubmit2 fp_vkQueueSubmit2 = nullptr;
//         PFN_vkQueueSubmit fp_vkQueueSubmit = nullptr;

//         // semaphores
//         PFN_vkCreateSemaphore fp_vkCreateSemaphore = nullptr;
//         PFN_vkDestroySemaphore fp_vkDestroySemaphore = nullptr;

//         // fences
//         PFN_vkWaitForFences fp_vkWaitForFences = nullptr;
//         PFN_vkResetFences fp_vkResetFences = nullptr;
//         PFN_vkCreateFence fp_vkCreateFence = nullptr;
//         PFN_vkDestroyFence fp_vkDestroyFence = nullptr;

//         //commandBuffers
//         PFN_vkBeginCommandBuffer fp_vkBeginCommandBuffer = nullptr;
//         // PFN_vkEndCommandBuffer
//         // PFN_vkAllocateCommandBuffers
//         // PFN_vkFreeCommandBuffers
//         // PFN_vkResetCommandBuffer

//         void init_instance_funcs(VkInstance inst)
//         {
//             instance = inst;
//             get_inst_proc_addr(fp_vkDestroyInstance, "vkDestroyInstance");
//             get_inst_proc_addr(fp_vkCreateDebugUtilsMessengerEXT, "vkCreateDebugUtilsMessengerEXT");
//             get_inst_proc_addr(fp_vkDestroyDebugUtilsMessengerEXT, "vkDestroyDebugUtilsMessengerEXT");
//             get_inst_proc_addr(fp_vkEnumeratePhysicalDevices, "vkEnumeratePhysicalDevices");

//             get_inst_proc_addr(fp_vkGetPhysicalDeviceFeatures, "vkGetPhysicalDeviceFeatures");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceFeatures2, "vkGetPhysicalDeviceFeatures2");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceFeatures2KHR, "vkGetPhysicalDeviceFeatures2KHR");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceProperties, "vkGetPhysicalDeviceProperties");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceQueueFamilyProperties, "vkGetPhysicalDeviceQueueFamilyProperties");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceMemoryProperties, "vkGetPhysicalDeviceMemoryProperties");
//             get_inst_proc_addr(fp_vkEnumerateDeviceExtensionProperties, "vkEnumerateDeviceExtensionProperties");
//             get_inst_proc_addr(fp_vkEnumerateDeviceLayerProperties, "vkEnumerateDeviceLayerProperties");

//             get_inst_proc_addr(fp_vkCreateDevice, "vkCreateDevice");
//             get_inst_proc_addr(fp_vkGetDeviceProcAddr, "vkGetDeviceProcAddr");

//             get_inst_proc_addr(fp_vkDestroySurfaceKHR, "vkDestroySurfaceKHR");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceSurfaceSupportKHR, "vkGetPhysicalDeviceSurfaceSupportKHR");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceSurfaceFormatsKHR, "vkGetPhysicalDeviceSurfaceFormatsKHR");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceSurfacePresentModesKHR, "vkGetPhysicalDeviceSurfacePresentModesKHR");
//             get_inst_proc_addr(fp_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

//             get_inst_proc_addr(fp_vkCmdBeginRendering, "vkCmdBeginRendering");
//             get_inst_proc_addr(fp_vkCmdEndRendering, "vkCmdEndRendering");
//             get_inst_proc_addr(fp_vkQueueSubmit2, "vkQueueSubmit2");
//             get_inst_proc_addr(fp_vkQueueSubmit, "vkQueueSubmit");

//             get_inst_proc_addr(fp_vkCreateSemaphore, "vkCreateSemaphore");
//             get_inst_proc_addr(fp_vkDestroySemaphore, "vkDestroySemaphore");

//             get_inst_proc_addr(fp_vkCreateFence, "vkCreateFence");
//             get_inst_proc_addr(fp_vkDestroyFence, "vkDestroyFence");
//             get_inst_proc_addr(fp_vkResetFences, "vkResetFences");
//             get_inst_proc_addr(fp_vkWaitForFences, "vkWaitForFences");
//         }
//     };
// }