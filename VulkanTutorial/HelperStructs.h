#ifndef HELPER_STRUCTS
#define HELPER_STRUCTS

#include <vulkan/vulkan.hpp>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	bool IsComplete();
};

#endif