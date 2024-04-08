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

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool copyCommandPool, VkQueue copyQueue, const std::vector<Vertex>& vertices) :
	m_PhysicalDevice{ physicalDevice },
	m_Device{ device },
	m_CopyCommandPool{ copyCommandPool },
	m_CopyQueue{ copyQueue },
	m_Vertices{ vertices },
	m_VertexBuffer{},
	m_VertexBufferMemory{}
{
	if (CreateVertexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create vertex buffer!");
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
	VkResult result{};

	VkBuffer stagingBuffer{};
	VkDeviceMemory stagingBufferMemory{};

	CreateBuffer
	(
		m_PhysicalDevice, 
		m_Device, 
		sizeof(Vertex) * m_Vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, 
		stagingBufferMemory
	);

	void* data{};
	result = vkMapMemory(m_Device, stagingBufferMemory, 0, sizeof(Vertex) * m_Vertices.size(), 0, &data);
	memcpy(data, m_Vertices.data(), sizeof(Vertex) * m_Vertices.size());
	vkUnmapMemory(m_Device, stagingBufferMemory);

	CreateBuffer
	(
		m_PhysicalDevice,
		m_Device,
		sizeof(Vertex) * m_Vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_VertexBuffer,
		m_VertexBufferMemory
	);

	CopyBuffer(m_Device, stagingBuffer, m_VertexBuffer, sizeof(Vertex) * m_Vertices.size(), m_CopyCommandPool, m_CopyQueue);

	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

	return result;
}