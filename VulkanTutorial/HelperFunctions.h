#ifndef  HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <vulkan/vulkan.hpp>

#include "HelperStructs.h"

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
bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);

// Find all the queue families we need
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

#endif