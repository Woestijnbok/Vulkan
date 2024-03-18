#ifndef  HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <vulkan/vulkan.hpp>

#include "HelperStructs.h"

struct GLFWwindow;

//Function to load and call the vkCreateDebugUtilsMessengerEXT function since it's not loaded automatically
VkResult CreateDebugUtilsMessengerEXT
(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
);

//Function to load and call the vkDestroyDebugUtilsMessengerEXT function since it's not loaded automatically
void DestroyDebugUtilsMessengerEXT
(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
);

// Function that will handle messages coming from our validation layer
VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
);

// Function that fills our VkDebugUtilsMessengerCreateInfoEXT struct
void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

// Checks if the gpu is suitable for the operations we want to do
bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*>& physicalExtensionNames);

// Find all the queue families we need
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

// Check if all required device extensions are enabled
bool DeviceExtenstionsPresent(VkPhysicalDevice device, std::vector<const char*>& physicalExtensionNames);

// Function that fills our VkDebugUtilsMessengerCreateInfoEXT struct
SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

// Will find the best possible format out of the given ones (swap chain)
VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

// Will find the best possible format out of the given ones (swap chain)
VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

// Will find the best possible swap extent this is about the resulutions of the images / surfaces in the swap chain
VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

#endif