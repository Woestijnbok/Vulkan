#ifndef TEXTURE
#define TEXTURE

#include <vulkan/vulkan.hpp>
#include <filesystem>

class Texture
{
public:
	Texture(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool copyCommandPool, VkQueue copyQueue, const std::filesystem::path& path);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture&&) = delete;
	Texture& operator=(Texture&&) = delete;

	VkImageView GetImageView() const;

private:
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkCommandPool m_CopyCommandPool;
	VkQueue m_CopyQueu;
	VkImage m_Image;						// VkImage is like a buffer but allows some easy of use for textures like 2D indexing
	VkDeviceMemory m_ImageMemory;
	VkImageView m_ImageView;

	void LoadTexture(const std::filesystem::path& path);
};

#endif