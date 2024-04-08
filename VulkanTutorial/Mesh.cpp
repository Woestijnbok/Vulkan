#include "Mesh.h"
#include "HelperFunctions.h"

VkVertexInputBindingDescription Vertex::GetBindingDescription()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputBindingDescription.html
	const VkVertexInputBindingDescription bindingDescription
	{
		0,									// binding
		sizeof(Vertex),						// stride
		VK_VERTEX_INPUT_RATE_VERTEX			// inputRate
	};

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::GetAttributeDescriptions()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html
	const std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions
	{
		VkVertexInputAttributeDescription
		{
			0,								// location
			0,								// binding
			VK_FORMAT_R32G32_SFLOAT,		// format
			offsetof(Vertex, Position)		// offset
		},
		VkVertexInputAttributeDescription
		{
			1,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, Color)
		}
	};

	return attributeDescriptions;
}

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<Vertex>& vertices) :
	m_PhysicalDevice{ physicalDevice },
	m_Device{ device },
	m_Vertices{ vertices },
	m_VertexBuffer{},
	m_VertexBufferMemory{}
{
	if (CreateVertexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create vertex buffer!");
	if (AllocateVertexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to allocate memory for vertex buffer!");
	if (BindVertexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to bind vertex buffer!");
}

Mesh::~Mesh()
{
	vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
}

const std::vector<Vertex>& Mesh::GetVertices() const
{
	return m_Vertices;
}

VkBuffer Mesh::GetVertexBuffer() const
{
	return m_VertexBuffer;
}

VkResult Mesh::CreateVertexBuffer()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBufferCreateInfo.html
	const VkBufferCreateInfo bufferCreateInfo
	{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,		// sType
		nullptr,									// pNext
		0,											// flags
		sizeof(Vertex) * m_Vertices.size(),			// size
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,			// usage
		VK_SHARING_MODE_EXCLUSIVE,					// sharingMode
		0,											// queueFamilyIndexCount
		0											// pQueueFamilyIndices
	};

	return vkCreateBuffer(m_Device, &bufferCreateInfo, nullptr, &m_VertexBuffer);
}

VkResult Mesh::AllocateVertexBuffer()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryRequirements.html
	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer, &memoryRequirements);

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryAllocateInfo.html
	const VkMemoryAllocateInfo memoryAllocateInfo
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,												// sType
		nullptr,																			// pNext
		memoryRequirements.size,															// allocationSize
		FindMemoryTypeIndex																	// memoryTypeIndex
		(
			m_PhysicalDevice,
			memoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		)
	};

	return vkAllocateMemory(m_Device, &memoryAllocateInfo, nullptr, &m_VertexBufferMemory);
}

VkResult Mesh::BindVertexBuffer()
{
	VkResult result{};

	result = vkBindBufferMemory(m_Device, m_VertexBuffer, m_VertexBufferMemory, 0);
	if (result != VK_SUCCESS) return result;

	void* data{};
	result = vkMapMemory(m_Device, m_VertexBufferMemory, 0, sizeof(Vertex) * m_Vertices.size(), 0, &data);
	memcpy(data, m_Vertices.data(), sizeof(Vertex) * m_Vertices.size());
	vkUnmapMemory(m_Device, m_VertexBufferMemory);

	return result;
}