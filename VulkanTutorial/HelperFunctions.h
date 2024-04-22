#ifndef  HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <vulkan/vulkan.hpp>
#include <filesystem>

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
void FillDebugMessengerCreateInfo
(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo
);

// Checks if the gpu is suitable for the operations we want to do
bool IsPhysicalDeviceSuitable
(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface, 
    std::vector<const char*>& physicalExtensionNames
);

// Find all the queue families we need
QueueFamilyIndices FindQueueFamilies
(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface
);

// Check if all required device extensions are enabled
bool DeviceExtenstionsPresent
(
    VkPhysicalDevice device, 
    std::vector<const char*>& physicalExtensionNames
);

// Function that fills our VkDebugUtilsMessengerCreateInfoEXT struct
SwapChainSupportDetails QuerySwapChainSupportDetails
(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface
);

// Will find the best possible format out of the given ones (swap chain)
VkSurfaceFormatKHR ChooseSurfaceFormat
(
    const std::vector<VkSurfaceFormatKHR>& availableFormats
);

// Will find the best possible format out of the given ones (swap chain)
VkPresentModeKHR ChoosePresentMode
(
    const std::vector<VkPresentModeKHR>& availablePresentModes
);

// Will find the best possible swap extent this is about the resulutions of the images / surfaces in the swap chain
VkExtent2D ChooseExtent
(
    const VkSurfaceCapabilitiesKHR& capabilities, 
    GLFWwindow* window
);

std::vector<char> LoadSPIRV
(
    const std::filesystem::path& path
);

VkShaderModule CreateShaderModule
(
    const std::vector<char>& buffer, 
    VkDevice device
);

uint32_t FindMemoryTypeIndex
(
    VkPhysicalDevice physicalDevice, 
    uint32_t typeFilter, 
    VkMemoryPropertyFlags properties
);

void CreateBuffer
(
    VkPhysicalDevice physicalDevice, 
    VkDevice device, 
    VkDeviceSize size, 
    VkBufferUsageFlags usage, 
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer, 
    VkDeviceMemory& bufferMemory
);


void CopyBuffer
(
    VkDevice device, 
    VkBuffer srcBuffer, 
    VkBuffer dstBuffer, 
    VkDeviceSize size, 
    VkCommandPool commandPool, 
    VkQueue queue
);

void CreateImage
(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkExtent2D size,
    VkFormat format,
    VkImageTiling tiling, 
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& memory, 
    uint32_t mipLevels
);

VkCommandBuffer BeginSingleTimeCommands
(
    VkDevice device, 
    VkCommandPool commandPool
);

void EndSingleTimeCommands
(
    VkDevice device, 
    VkCommandPool commandpool, 
    VkQueue queue, 
    VkCommandBuffer commandBuffer
);

void TransitionImageLayout
(
    VkDevice device, 
    VkCommandPool commandpool, 
    VkQueue queue, 
    VkImage image, 
    VkFormat format, 
    VkImageLayout oldLayout, 
    VkImageLayout newLayout, 
    uint32_t mipLevels
);

void CopyBufferToImage
(
    VkDevice device, 
    VkCommandPool commandpool, 
    VkQueue queue, 
    VkBuffer buffer, 
    VkImage image, 
    uint32_t width, 
    uint32_t height
);

VkImageView CreateImageView
(
    VkDevice device,
    VkImage image, 
    VkFormat format, 
    VkImageAspectFlags aspectFlags, 
    uint32_t mipLevels
);

VkFormat FindSupportedFormat
(
    VkPhysicalDevice physicalDevice, 
    const std::vector<VkFormat>& candidates, 
    VkImageTiling tiling, 
    VkFormatFeatureFlags features
);

VkFormat FindDepthFormat
(
    VkPhysicalDevice physicalDevice
);

bool HasStencilComponent
(
    VkFormat format
);

void GenerateMipmaps
(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue,
    VkImage image,
    VkFormat imageFormat,
    int32_t texWidth,
    int32_t texHeight,
    uint32_t mipLevels
);

#endif