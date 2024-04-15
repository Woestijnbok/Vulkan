#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>
#include <unordered_map>

#include "Mesh.h"
#include "HelperFunctions.h"

bool Vertex::operator==(const Vertex& other) const
{
	return (Position == other.Position) and (Color == other.Color) and (TextureCoordinates == other.TextureCoordinates);
}

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

std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html
	const std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions
	{
		// Position
		VkVertexInputAttributeDescription
		{
			0,								// location
			0,								// binding
			VK_FORMAT_R32G32B32_SFLOAT,		// format
			offsetof(Vertex, Position)		// offset
		},
		// Color
		VkVertexInputAttributeDescription
		{
			1,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, Color)
		},
		// Texture coordinates
		VkVertexInputAttributeDescription
		{
			2,	
			0,	
			VK_FORMAT_R32G32_SFLOAT,	
			offsetof(Vertex, TextureCoordinates)	
		}
	};

	return attributeDescriptions;
}

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool copyCommandPool, VkQueue copyQueue, const std::filesystem::path& path) :
	m_PhysicalDevice{ physicalDevice },
	m_Device{ device },
	m_CopyCommandPool{ copyCommandPool },
	m_CopyQueue{ copyQueue },
	m_Vertices{},
	m_VertexBuffer{},
	m_VertexBufferMemory{},
	m_Indices{},
	m_IndexBuffer{},
	m_IndexBufferMemory{},
	m_ModelMatrix{ 1.0f }
{
	LoadMesh(path);
	if (CreateVertexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create vertex buffer!");
	if (CreateIndexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create index buffer!");
}

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool copyCommandPool, VkQueue copyQueue, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) :
	m_PhysicalDevice{ physicalDevice },
	m_Device{ device },
	m_CopyCommandPool{ copyCommandPool },
	m_CopyQueue{ copyQueue },
	m_Vertices{ vertices },
	m_VertexBuffer{},
	m_VertexBufferMemory{},
	m_Indices{ indices },
	m_IndexBuffer{},
	m_IndexBufferMemory{},
	m_ModelMatrix{ 1.0f }
{
	if (CreateVertexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create vertex buffer!");
	if (CreateIndexBuffer() != VK_SUCCESS) throw std::runtime_error("Failed to create index buffer!");
}

Mesh::~Mesh()
{
	// Vertex buffer
	vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

	// Index buffer
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

const std::vector<uint32_t> Mesh::GetIndices() const
{
	return m_Indices;
}

VkBuffer Mesh::GetIndexBuffer() const
{
	return m_IndexBuffer;
}

glm::mat4 Mesh::GetModelMatrix() const
{
	return m_ModelMatrix;
}

void Mesh::SetModelMatrix(const glm::mat4& matrix)
{
	m_ModelMatrix = matrix;
}

VkResult Mesh::CreateVertexBuffer()
{
	VkResult result{};

	VkBuffer vertexStagingBuffer{};
	VkDeviceMemory vertexStagingBufferMemory{};

	CreateBuffer
	(
		m_PhysicalDevice, 
		m_Device, 
		sizeof(Vertex) * m_Vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		vertexStagingBuffer, 
		vertexStagingBufferMemory
	);

	void* data{};
	result = vkMapMemory(m_Device, vertexStagingBufferMemory, 0, sizeof(Vertex) * m_Vertices.size(), 0, &data);
	memcpy(data, m_Vertices.data(), sizeof(Vertex) * m_Vertices.size());
	vkUnmapMemory(m_Device, vertexStagingBufferMemory);

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

	CopyBuffer(m_Device, vertexStagingBuffer, m_VertexBuffer, sizeof(Vertex) * m_Vertices.size(), m_CopyCommandPool, m_CopyQueue);

	vkDestroyBuffer(m_Device, vertexStagingBuffer, nullptr);
	vkFreeMemory(m_Device, vertexStagingBufferMemory, nullptr);

	return result;
}

VkResult Mesh::CreateIndexBuffer()
{
	VkResult result{};

	const size_t bufferSize{ sizeof(uint32_t) * m_Indices.size() };

	VkBuffer indexStagingBuffer{};
	VkDeviceMemory indexStagingBufferMemory{};

	CreateBuffer
	(
		m_PhysicalDevice,
		m_Device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indexStagingBuffer,
		indexStagingBufferMemory
	);

	void* data{};
	result = vkMapMemory(m_Device, indexStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_Indices.data(), bufferSize);
	vkUnmapMemory(m_Device, indexStagingBufferMemory);

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

	CopyBuffer(m_Device, indexStagingBuffer, m_IndexBuffer, bufferSize, m_CopyCommandPool, m_CopyQueue);

	return result;
}

void Mesh::LoadMesh(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path)) throw std::runtime_error("Invalid texture file path given!");

	tinyobj::attrib_t attributes{};
	std::vector<tinyobj::shape_t> shapes{};
	std::vector<tinyobj::material_t> materials{};
	std::string error{};

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, path.string().c_str())) 
	{
		throw std::runtime_error(error);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex{};

			vertex.Position = glm::vec3
			{
				attributes.vertices[3 * index.vertex_index + 0],
				attributes.vertices[3 * index.vertex_index + 1],
				attributes.vertices[3 * index.vertex_index + 2]
			};

			vertex.TextureCoordinates = glm::vec2
			{
				attributes.texcoords[2 * index.texcoord_index + 0],
				1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.Color = glm::vec3{ 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) 
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
				m_Vertices.push_back(vertex);
			}

			m_Indices.push_back(uniqueVertices[vertex]);
		}
	}
}