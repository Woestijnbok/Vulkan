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

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool copyCommandPool, VkQueue copyQueue, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) :
	m_PhysicalDevice{ physicalDevice },
	m_Device{ device },
	m_CopyCommandPool{ copyCommandPool },
	m_CopyQueue{ copyQueue },
	m_Vertices{ vertices },
	m_VertexStagingBuffer{},
	m_VertexBuffer{},
	m_VertexStagingBufferMemory{},
	m_VertexBufferMemory{},
	m_Indices{ indices },
	m_IndexStagingBuffer{},
	m_IndexBuffer{},
	m_IndexStagingBufferMemory{},
	m_IndexBufferMemory{}
{
	if (CreateVertexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create vertex buffer!");
	if (CreateIndexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create index buffer!");
}

Mesh::~Mesh()
{
	// Vertex buffer
	vkDestroyBuffer(m_Device, m_VertexStagingBuffer, nullptr);
	vkFreeMemory(m_Device, m_VertexStagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

	// Index buffer
	vkDestroyBuffer(m_Device, m_IndexStagingBuffer, nullptr);
	vkFreeMemory(m_Device, m_IndexStagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
	vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);
}

const std::vector<Vertex>& Mesh::GetVertices() const
{
	return m_Vertices;
}

VkBuffer Mesh::GetVertexBuffer() const
{
	return m_VertexBuffer;
}

const std::vector<uint16_t> Mesh::GetIndices() const
{
	return m_Indices;
}
VkBuffer Mesh::GetIndexBuffer() const
{
	return m_IndexBuffer;
}

VkResult Mesh::CreateVertexBuffer()
{
	VkResult result{};

	CreateBuffer
	(
		m_PhysicalDevice, 
		m_Device, 
		sizeof(Vertex) * m_Vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		m_VertexStagingBuffer, 
		m_VertexStagingBufferMemory
	);

	void* data{};
	result = vkMapMemory(m_Device, m_VertexStagingBufferMemory, 0, sizeof(Vertex) * m_Vertices.size(), 0, &data);
	memcpy(data, m_Vertices.data(), sizeof(Vertex) * m_Vertices.size());
	vkUnmapMemory(m_Device, m_VertexStagingBufferMemory);

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

	CopyBuffer(m_Device, m_VertexStagingBuffer, m_VertexBuffer, sizeof(Vertex) * m_Vertices.size(), m_CopyCommandPool, m_CopyQueue);

	return result;
}

VkResult Mesh::CreateIndexBuffer()
{
	VkResult result{};

	const size_t bufferSize{ sizeof(uint16_t) * m_Indices.size() };

	CreateBuffer
	(
		m_PhysicalDevice,
		m_Device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_IndexStagingBuffer,
		m_IndexStagingBufferMemory
	);

	void* data{};
	result = vkMapMemory(m_Device, m_IndexStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_Indices.data(), bufferSize);
	vkUnmapMemory(m_Device, m_IndexStagingBufferMemory);

	CreateBuffer
	(
		m_PhysicalDevice,
		m_Device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_IndexBuffer,
		m_IndexBufferMemory
	);

	CopyBuffer(m_Device, m_IndexStagingBuffer, m_IndexBuffer, bufferSize, m_CopyCommandPool, m_CopyQueue);

	return result;
}