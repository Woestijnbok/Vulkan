#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#include "Texture.h"
#include "HelperFunctions.h"

Texture::Texture(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool copyCommandPool, VkQueue copyQueue, const std::filesystem::path& path) :
	m_PhysicalDevice{ physicalDevice },
	m_Device{ device },
	m_CopyCommandPool{ copyCommandPool },
	m_CopyQueu{ copyQueue },
	m_Image{},
	m_ImageMemory{},
	m_ImageView{}
{
	LoadTexture(path);
}

Texture::~Texture()
{
	vkDestroyImageView(m_Device, m_ImageView, nullptr);
	vkDestroyImage(m_Device, m_Image, nullptr);
	vkFreeMemory(m_Device, m_ImageMemory, nullptr);
}

VkImageView Texture::GetImageView() const
{
	return m_ImageView;
}

uint32_t Texture::GetMipLevels() const
{
	return m_MipLevels;
}

void Texture::LoadTexture(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path)) throw std::runtime_error("Invalid texture file path given!");

	int textureWidth{}, textureHeight{}, textureChannels{};
	stbi_uc* pixels{ stbi_load(path.string().c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha) };
	m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
	const VkDeviceSize imageSize{ VkDeviceSize(textureWidth) * VkDeviceSize(textureHeight) * 4 };

	if (!pixels) throw std::runtime_error("failed to load texture image!");

	VkBuffer stagingPixelBuffer{};
	VkDeviceMemory stagingPixelBufferMemory{};

	CreateBuffer
	(
		m_PhysicalDevice,
		m_Device,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingPixelBuffer,
		stagingPixelBufferMemory
	);

	void* data{};
	vkMapMemory(m_Device, stagingPixelBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_Device, stagingPixelBufferMemory);
	stbi_image_free(pixels);

	CreateImage
	(
		m_PhysicalDevice,
		m_Device,
		VkExtent2D{ uint32_t(textureWidth), uint32_t(textureHeight) },
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_Image,
		m_ImageMemory,
		m_MipLevels,
		VK_SAMPLE_COUNT_1_BIT
	);

	TransitionImageLayout(m_Device, m_CopyCommandPool, m_CopyQueu, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
	CopyBufferToImage(m_Device, m_CopyCommandPool, m_CopyQueu, stagingPixelBuffer, m_Image, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight));
	
	GenerateMipmaps(m_PhysicalDevice, m_Device, m_CopyCommandPool, m_CopyQueu, m_Image, VK_FORMAT_R8G8B8A8_SRGB, textureWidth, textureHeight, m_MipLevels);

	vkDestroyBuffer(m_Device, stagingPixelBuffer, nullptr);
	vkFreeMemory(m_Device, stagingPixelBufferMemory, nullptr);

	m_ImageView = CreateImageView(m_Device, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);	
}