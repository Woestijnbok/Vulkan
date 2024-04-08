#ifndef HELPER_STRUCTS
#define HELPER_STRUCTS

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#include <glm/glm.hpp>

struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	bool IsComplete();
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

#endif