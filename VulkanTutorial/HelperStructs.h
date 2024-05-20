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
	alignas(16) glm::mat4 ModelMatrix;
	alignas(16) glm::mat4 ViewMatrix;
	alignas(16) glm::mat4 ProjectionMatrix;
	alignas(16) glm::vec3 CameraPosition;
};

#endif