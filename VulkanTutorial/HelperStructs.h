#ifndef HELPER_STRUCTS
#define HELPER_STRUCTS

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#include <glm/glm.hpp>

struct QueueFamilyIndices final
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	bool IsComplete();
};

struct SwapChainSupportDetails final
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

struct UniformBufferObject final
{
	glm::mat4 ModelMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};

#endif