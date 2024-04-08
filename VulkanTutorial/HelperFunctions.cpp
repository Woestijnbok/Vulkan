#include <iostream>
#include <iomanip>
#include <GLFW/glfw3.h>
#include <fstream>

#include "HelperFunctions.h"

VkResult CreateDebugUtilsMessengerEXT
(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT
(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/
)
{
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        if (messageType > VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }
    }



    return VK_FALSE;
}

void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerCreateInfoEXT.html
    createInfo = VkDebugUtilsMessengerCreateInfoEXT{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = &MessageCallback;
}

bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*>& physicalExtensionNames)
{
    QueueFamilyIndices indices{ FindQueueFamilies(device, surface) };

    bool queueFamiliesPresent{ indices.IsComplete() };

    bool deviceExtensionPresent{ DeviceExtenstionsPresent(device, physicalExtensionNames) };

    bool swapChainDetailsPresent{ false };
    if (deviceExtensionPresent)
    {
        // Is okay for now if there is one supported format and present mode for given device and surface;
        SwapChainSupportDetails swapChainDetails{ QuerySwapChainSupportDetails(device, surface) };
        swapChainDetailsPresent = !swapChainDetails.Formats.empty() && !swapChainDetails.PresentModes.empty();
    }
    
    return queueFamiliesPresent and deviceExtensionPresent and swapChainDetailsPresent;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices{};
    
    uint32_t queueFamilyCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties{ queueFamilyCount };
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

    int i{};
    for (const auto& queueFamilyProperty : queueFamilyProperties)
    {
        // Checking draw queues are supported
        if ((not indices.GraphicsFamily.has_value()) and (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            indices.GraphicsFamily = i;
        }

        // Checking if present queues are supported
        VkBool32 presentQueueFamilySupport{};
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentQueueFamilySupport);
        if (presentQueueFamilySupport)
        {
            indices.PresentFamily = i;
        }

        if (indices.IsComplete()) break;

        ++i;
    }

    return indices;
}

bool DeviceExtenstionsPresent(VkPhysicalDevice device, std::vector<const char*>& physicalExtensionNames)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    bool extensionsPresent{ true };
    std::cout << std::endl << "-----Physical Device Extensions-----" << std::endl;
    for (const auto& extensionName : physicalExtensionNames)
    {
        std::cout << std::setw(40) << std::left << extensionName;

        extensions.at(0).extensionName;

        if (std::ranges::find_if(extensions,
            [&extensionName](const auto& extension) -> bool
            {
                return strcmp(extensionName, extension.extensionName) == 0;
            }
        ) != extensions.end())
        {
            std::cout << std::setw(40) << std::left << "PRESENT";
        }
        else
        {
            extensionsPresent = false;
            std::cout << std::setw(40) << std::left << "NOT PRESENT";
        }

        std::cout << std::endl;
    }
    std::cout << std::endl;

    return extensionsPresent;
}

SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details{};

    // Capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

    // Surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) 
    {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
    }

    // Present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) 
    {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        // format with r,g,b,a channel taking each 8bits & srgb
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            return availableFormat;
        }
    }

    return availableFormats.at(0);
}

VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) 
    {
        // Is the triple buffering principle
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
        {
            return availablePresentMode;
        }
    }

    // Is always present
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    // Does the window manager not allow to have different resulotion in the window then the images / surfaces in the swapchain
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

std::vector<char> LoadSPIRV(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) throw std::runtime_error("Specified path does not exist!");

    std::ifstream file{ path, std::ios::ate | std::ios::binary };

    if(!file.is_open()) throw std::runtime_error("Failed to open file");

    const size_t fileSize{ size_t(file.tellg()) };
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule CreateShaderModule(const std::vector<char>& buffer, VkDevice device)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = buffer.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shaderModule{};
    if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) throw std::runtime_error("Failed to create shader module!");

    return shaderModule;
}

uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i{ 0 }; i < memoryProperties.memoryTypeCount; ++i)
    {
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryType.html
        if ((typeFilter & (1 << i)) and ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferCreateInfo.html
    const VkBufferCreateInfo bufferCreateInfo
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,		// sType
        nullptr,									// pNext
        0,											// flags
        size,			                            // size
        usage,			                            // usage
        VK_SHARING_MODE_EXCLUSIVE,					// sharingMode
        0,											// queueFamilyIndexCount
        0											// pQueueFamilyIndices
    };

    if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create buffer!");
    }

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryRequirements.html
    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryAllocateInfo.html
    const VkMemoryAllocateInfo memoryAllocateInfo
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,												// sType
        nullptr,																			// pNext
        memoryRequirements.size,															// allocationSize
        FindMemoryTypeIndex																	// memoryTypeIndex
        (
            physicalDevice,
            memoryRequirements.memoryTypeBits,
            properties
        )
    };

    if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void CopyBuffer(VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool, VkQueue queue) 
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferAllocateInfo.html
    const VkCommandBufferAllocateInfo commandBufferAllocateInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,     // sType
        nullptr,                                            // pNext
        commandPool,                                        // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                    // level
        1                                                   // commandBufferCount
    };

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBuffer.html
    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferBeginInfo.html
    const VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // sType
        nullptr,                                            // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        // flags
        nullptr                                             // pInheritanceInfo
    };
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferCopy.html
    const VkBufferCopy bufferCopy
    {
        0,          // srcOffset
        0,          // dstOffset
        size        // size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

    vkEndCommandBuffer(commandBuffer);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubmitInfo.html
    const VkSubmitInfo submitInfo
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,      // sType
        nullptr,                            // pNext
        0,                                  // waitSemaphoreCount
        nullptr,                            // pWaitSemaphores
        nullptr,                            // pWaitDstStageMask
        1,                                  // commandBufferCount
        &commandBuffer,                     // pCommandBuffers
        0,                                  // signalSemaphoreCount
        nullptr                             // pSignalSemaphores
    };

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}