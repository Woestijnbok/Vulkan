#include <iostream>
#include <iomanip>
#include <glfw3.h>
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

void FillDebugMessengerCreateInfo
(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo
)
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerCreateInfoEXT.html
    createInfo = VkDebugUtilsMessengerCreateInfoEXT{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = &MessageCallback;
}

bool IsPhysicalDeviceSuitable
(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface, 
    std::vector<const char*>& physicalExtensionNames
)
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

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    vkGetPhysicalDeviceFeatures(device, &physicalDeviceFeatures);
    
    return queueFamiliesPresent and deviceExtensionPresent and swapChainDetailsPresent and physicalDeviceFeatures.samplerAnisotropy;
}

QueueFamilyIndices FindQueueFamilies
(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface
)
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

bool DeviceExtenstionsPresent
(
    VkPhysicalDevice device, 
    std::vector<const char*>& physicalExtensionNames
)
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

SwapChainSupportDetails QuerySwapChainSupportDetails
(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface
)
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

VkSurfaceFormatKHR ChooseSurfaceFormat
(
    const std::vector<VkSurfaceFormatKHR>& availableFormats
)
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

VkPresentModeKHR ChoosePresentMode
(
    const std::vector<VkPresentModeKHR>& availablePresentModes
)
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

VkExtent2D ChooseExtent
(
    const VkSurfaceCapabilitiesKHR& capabilities, 
    GLFWwindow* window
)
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

std::vector<char> LoadSPIRV
(
    const std::filesystem::path& path
)
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

VkShaderModule CreateShaderModule
(
    const std::vector<char>& buffer, VkDevice device
)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = buffer.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shaderModule{};
    if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) throw std::runtime_error("Failed to create shader module!");

    return shaderModule;
}

uint32_t FindMemoryTypeIndex
(
    VkPhysicalDevice physicalDevice, 
    uint32_t typeFilter, 
    VkMemoryPropertyFlags properties
)
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

void CreateBuffer
(
    VkPhysicalDevice physicalDevice, 
    VkDevice device, 
    VkDeviceSize size, 
    VkBufferUsageFlags usage, 
    VkMemoryPropertyFlags properties, 
    VkBuffer& buffer, 
    VkDeviceMemory& bufferMemory
) 
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

void CopyBuffer
(
    VkDevice device, 
    VkBuffer srcBuffer, 
    VkBuffer dstBuffer, 
    VkDeviceSize size,
    VkCommandPool commandPool, 
    VkQueue queue
) 
{
    VkCommandBuffer commandBuffer{ BeginSingleTimeCommands(device, commandPool) };

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferCopy.html
    const VkBufferCopy bufferCopy
    {
        0,          // srcOffset
        0,          // dstOffset
        size        // size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

    EndSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

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
    uint32_t mipLevels,
    VkSampleCountFlagBits sampleCount
)
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageCreateInfo.html
    const VkImageCreateInfo imageCreateInfo
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                                        // sType
        nullptr,                                                                    // pNext
        0,                                                                          // flags
        VK_IMAGE_TYPE_2D,                                                           // imageType
        format,                                                                     // format
        VkExtent3D{ size.width, size.height, 1 },                                   // extent
        mipLevels,                                                                  // mipLevels
        1,                                                                          // arrayLayers
        sampleCount,                                                                // samples
        tiling,                                                                     // tiling
        usage,                                                                      // usage
        VK_SHARING_MODE_EXCLUSIVE,                                                  // sharingMode
        0,                                                                          // queueFamilyIndexCount
        nullptr,                                                                    // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED                                                   // initialLayout
    };

    if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image!");
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryAllocateInfo.html
    const VkMemoryAllocateInfo memoryAllocateInfo
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,             // sType                                       
        nullptr,                                            // pNext
        memoryRequirements.size,                            // allocationSize
        FindMemoryTypeIndex                                 // memoryTypeIndex
        (
            physicalDevice,
            memoryRequirements.memoryTypeBits,
            properties  
        )
    };

    if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, memory, 0);
}

VkCommandBuffer BeginSingleTimeCommands
(
    VkDevice device, 
    VkCommandPool commandPool
) 
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferAllocateInfo.html
    const VkCommandBufferAllocateInfo commandBufferAllocateInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,             // sType
        nullptr,                                                    // pNext
        commandPool,                                                // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                            // level
        1                                                           // commandBufferCount
    };

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferBeginInfo.html
    const VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,                // sType
        nullptr,                                                    // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,                // flags
        nullptr                                                     // pInheritanceInfo
    };

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

void EndSingleTimeCommands
(
    VkDevice device,
    VkCommandPool commandpool, 
    VkQueue queue, 
    VkCommandBuffer commandBuffer
) 
{
    vkEndCommandBuffer(commandBuffer);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubmitInfo.html
    VkSubmitInfo submitInfo
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,              // sType
        nullptr,                                    // pNext
        0,                                          // waitSemaphoreCount
        nullptr,                                    // pWaitSemaphores
        nullptr,                                    // pWaitDstStageMask
        1,                                          // commandBufferCount
        &commandBuffer,                             // pCommandBuffers
        0,                                          // signalSemaphoreCount
        nullptr                                     // pSignalSempahores
    };

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandpool, 1, &commandBuffer);
}

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
)
{
    VkCommandBuffer commandBuffer{ BeginSingleTimeCommands(device, commandpool) };

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageMemoryBarrier.html
    VkImageMemoryBarrier imageMemoryBarrier
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // sType
        nullptr,                                            // pNext
        0,                                                  // srcAccesMask
        0,                                                  // dstAccessMask
        oldLayout,                                          // oldLayout
        newLayout,                                          // newLayout
        VK_QUEUE_FAMILY_IGNORED,                            // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                            // dstQueueFamilyIndex
        image,                                              // image
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageSubresourceRange.html
        VkImageSubresourceRange                             // subresourceRange
        {
            VK_IMAGE_ASPECT_COLOR_BIT,          // aspectMask
            0,                                  // baseMipLevel
            mipLevels,                          // levelCount
            0,                                  // baseArrayLayer
            1                                   // layerCount
        }   
    };

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
    {
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStencilComponent(format)) 
        {
            imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else 
    {
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage{};
    VkPipelineStageFlags destinationStage{};

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
    {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else 
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier
    (
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier
    );

    EndSingleTimeCommands(device, commandpool, queue, commandBuffer);

    format;
}

void CopyBufferToImage
(
    VkDevice device, 
    VkCommandPool commandpool,
    VkQueue queue, 
    VkBuffer buffer, 
    VkImage image, 
    uint32_t width, 
    uint32_t height
) 
{
    VkCommandBuffer commandBuffer{ BeginSingleTimeCommands(device, commandpool ) };

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferImageCopy.html
    const VkBufferImageCopy bufferImageCopy
    {
        0,                                                                      // bufferOffset
        0,                                                                      // bufferRowLength
        0,                                                                      // bufferImageHeight
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageSubresourceLayers.html
        VkImageSubresourceLayers                                                // imageSubresource
        {
            VK_IMAGE_ASPECT_COLOR_BIT,          // aspectMask
            0,                                  // mipLevel
            0,                                  // baseArrayLayer
            1                                   // layerCount
        },
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkOffset3D.html
        VkOffset3D                                                              // imageOffset
        { 
            0,                                  // x
            0,                                  // y
            0                                   // z
        },
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkExtent3D.html
        VkExtent3D                                                              // imageExtent
        { 
            width,                              // width
            height,                             // height
            1                                   // depth
        }
    };

    vkCmdCopyBufferToImage
    (
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &bufferImageCopy
    );

    EndSingleTimeCommands(device, commandpool, queue, commandBuffer);
}

VkImageView CreateImageView
(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags, 
    uint32_t mipLevels
)
{
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageViewCreateInfo.html
    const VkImageViewCreateInfo imageViewcreateInfo
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,				// sType
        nullptr,												// pNext
        0,														// flags
        image,												    // image
        VK_IMAGE_VIEW_TYPE_2D,									// viewType
        format,								                    // format
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkComponentMapping.html
        VkComponentMapping										// components
        {
            VK_COMPONENT_SWIZZLE_IDENTITY,		// r
            VK_COMPONENT_SWIZZLE_IDENTITY,		// g
            VK_COMPONENT_SWIZZLE_IDENTITY,		// b
            VK_COMPONENT_SWIZZLE_IDENTITY		// a
        },
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageSubresourceRange.html
        VkImageSubresourceRange									// subresourceRange
        {
            aspectFlags,			            // aspectMask
            0,									// baseMipLevel
            mipLevels,							// levelCount
            0,									// baseArrayLayer
            1									// layerCount
        }
    };

    VkImageView imageView{};
    if (vkCreateImageView(device, &imageViewcreateInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture image view!");
    }

    return imageView;
}

VkFormat FindSupportedFormat
(
    VkPhysicalDevice physicalDevice, 
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, 
    VkFormatFeatureFlags features
) 
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties formatProperties{};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

        if ((tiling == VK_IMAGE_TILING_LINEAR) and ((formatProperties.linearTilingFeatures & features) == features))
        {
            return format;
        }
        else if ((tiling == VK_IMAGE_TILING_OPTIMAL) and ((formatProperties.optimalTilingFeatures & features) == features))
        {
            return format;
        }
    }
   
    throw std::runtime_error("Failed to find supported format!");
}

VkFormat FindDepthFormat
(
    VkPhysicalDevice physicalDevice
)
{
    return FindSupportedFormat
    (
        physicalDevice,
        std::vector<VkFormat>{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool HasStencilComponent
(
    VkFormat format
) 
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

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
)
{
    VkFormatProperties formatProperties{};
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
    {
        throw std::runtime_error("texture image format does not support linear blitting!"); 
    }

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageMemoryBarrier.html
    VkImageMemoryBarrier imageMemoryBarrier
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,                         // sType
        nullptr,                                                        // pNext
        0,                                                              // srcAccessMask
        0,                                                              // dstAccessMask
        VK_IMAGE_LAYOUT_UNDEFINED,                                      // oldLayout
        VK_IMAGE_LAYOUT_UNDEFINED,                                      // newLayout
        VK_QUEUE_FAMILY_IGNORED,                                        // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                                        // dstQueueFamilyIndex
        image,                                                          // image
        VkImageSubresourceRange                                         // subresourceRange
        {
            VK_IMAGE_ASPECT_COLOR_BIT,      // aspectMask
            0,                              // baseMipLevel
            1,                              // levelCount
            0,                              // baseArrayLayer
            1                               // layerCount
        }
    };

    int32_t mipWidth{ texWidth };
    int32_t mipHeight{ texHeight };

    for (uint32_t i{ 1 }; i < mipLevels; ++i)
    {
        imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;    
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;    
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;        
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; 

        vkCmdPipelineBarrier
        (
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier
        );

        const VkImageBlit imageBlit
        {
            VkImageSubresourceLayers                            // srcSubresource
            {
                VK_IMAGE_ASPECT_COLOR_BIT,      // aspectMask
                i - 1,                          // mipLevel
                0,                              // baseArrayLayer
                1                               // layerCount
            },                                                 
            {                                                   // srcOffsets[2]
                VkOffset3D                                       
                {
                    0,      // x
                    0,      // y
                    0       // z
                },
                VkOffset3D
                {
                    mipWidth,
                    mipHeight,
                    1
                }
            },
            VkImageSubresourceLayers                            // dstSubresource
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                i,
                0,
                1
            },                                                    
            {                                                   // dstOffsets[2]
                VkOffset3D
                {
                    0,
                    0,
                    0
                },
                VkOffset3D
                {
                    (mipWidth > 1) ? mipWidth / 2 : 1, 
                    (mipHeight > 1) ? mipHeight / 2 : 1, 
                    1
                }
            }
        };

        vkCmdBlitImage  
        (
            commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,    
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    
            1, &imageBlit,      
            VK_FILTER_LINEAR    
        );

        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;    
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;    
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT; 
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;   

        vkCmdPipelineBarrier
        (
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier
        );

        if (mipWidth > 1) mipWidth /= 2;    
        if (mipHeight > 1) mipHeight /= 2;  
    }

    imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;   
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;    
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;    
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;    
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;   

    vkCmdPipelineBarrier
    (
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,   
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier  
    );

    EndSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

VkSampleCountFlagBits GetMaxUsableSampleCount
(
    VkPhysicalDevice device
)
{
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

    // We have to take both the color and depth buffer into acount
    const VkSampleCountFlags Samplecount{ physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts };

    // Get the highest possible sample count
    if (Samplecount & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    else if (Samplecount & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    else if (Samplecount & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    else if (Samplecount & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    else if (Samplecount & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    else if (Samplecount & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
    else return VK_SAMPLE_COUNT_1_BIT;
}