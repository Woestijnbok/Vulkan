#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Mesh.h"
#include "HelperFunctions.h"
#include "Camera.h"

bool Vertex::operator==(const Vertex& other) const
{
	return (Position == other.Position) and (Color == other.Color) and (TextureCoordinates == other.TextureCoordinates)
		and (Tangent == other.Tangent);
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

std::array<VkVertexInputAttributeDescription, 5> Vertex::GetAttributeDescriptions()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html
	const std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions
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
		},
		// Normal
		VkVertexInputAttributeDescription
		{
			3,								
			0,								
			VK_FORMAT_R32G32B32_SFLOAT,		
			offsetof(Vertex, Normal)		
		},
		// Tangent
		VkVertexInputAttributeDescription
		{
			4,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(Vertex, Tangent)
		}
	};

	return attributeDescriptions;
}

size_t std::hash<Vertex>::operator()(const Vertex& vertex) const
{
	// Combine hash values of Position, Color, and TextureCoordinates
	size_t hashValue = 0;

	// Hash Position
	hash_combine(hashValue, vertex.Position.x);	
	hash_combine(hashValue, vertex.Position.y);
	hash_combine(hashValue, vertex.Position.z);

	// Hash Color
	hash_combine(hashValue, vertex.Color.r);
	hash_combine(hashValue, vertex.Color.g);
	hash_combine(hashValue, vertex.Color.b);

	// Hash TextureCoordinates
	hash_combine(hashValue, vertex.TextureCoordinates.x);
	hash_combine(hashValue, vertex.TextureCoordinates.y);

	// Hash Normal
	hash_combine(hashValue, vertex.Normal.x);
	hash_combine(hashValue, vertex.Normal.y);
	hash_combine(hashValue, vertex.Normal.z);

	// Hash Tangent
	hash_combine(hashValue, vertex.Tangent.x);
	hash_combine(hashValue, vertex.Tangent.y);
	hash_combine(hashValue, vertex.Tangent.z);

	return hashValue;
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
	m_ModelMatrix{ 1.0f },
	m_Rotate{ true }
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

void Mesh::Update(std::chrono::duration<float> elapsedSeconds)
{
	if (!m_Rotate) return;

	// Calculate total rotation angle for 2 seconds
	constexpr float totalRotationDegrees = 360.0f; // Rotate fully in 2 seconds
	float rotationSpeed = totalRotationDegrees / 4.0f; // 180 degrees per second for 2 seconds

	// Calculate rotation angle based on elapsed time
	float rotationAngle = rotationSpeed * elapsedSeconds.count();

	// Apply rotation to the model matrix
	m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotationAngle), g_WorldRight);
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

void Mesh::SwitchRotate()
{
	m_Rotate = !m_Rotate;
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

	vkDestroyBuffer(m_Device, indexStagingBuffer, nullptr);
	vkFreeMemory(m_Device, indexStagingBufferMemory, nullptr);

	return result;
}

void Mesh::LoadMesh(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path)) throw std::runtime_error("Invalid texture file path given!");

	tinyobj::attrib_t attributes{};
	std::vector<tinyobj::shape_t> shapes{};
	std::vector<tinyobj::material_t> materials{};
	std::string error{};

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, path.string().c_str())) {
		throw std::runtime_error(error);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
			// Indices for the triangle
			tinyobj::index_t idx0 = shape.mesh.indices[i];
			tinyobj::index_t idx1 = shape.mesh.indices[i + 1];
			tinyobj::index_t idx2 = shape.mesh.indices[i + 2];

			// Vertices of the triangle
			Vertex vertex0 = {};
			Vertex vertex1 = {};
			Vertex vertex2 = {};

			vertex0.Position = glm::vec3(
				attributes.vertices[3 * idx0.vertex_index + 0],
				attributes.vertices[3 * idx0.vertex_index + 1],
				attributes.vertices[3 * idx0.vertex_index + 2]
			);
			vertex1.Position = glm::vec3(
				attributes.vertices[3 * idx1.vertex_index + 0],
				attributes.vertices[3 * idx1.vertex_index + 1],
				attributes.vertices[3 * idx1.vertex_index + 2]
			);
			vertex2.Position = glm::vec3(
				attributes.vertices[3 * idx2.vertex_index + 0],
				attributes.vertices[3 * idx2.vertex_index + 1],
				attributes.vertices[3 * idx2.vertex_index + 2]
			);

			vertex0.TextureCoordinates = glm::vec2(
				attributes.texcoords[2 * idx0.texcoord_index + 0],
				1.0f - attributes.texcoords[2 * idx0.texcoord_index + 1]
			);
			vertex1.TextureCoordinates = glm::vec2(
				attributes.texcoords[2 * idx1.texcoord_index + 0],
				1.0f - attributes.texcoords[2 * idx1.texcoord_index + 1]
			);
			vertex2.TextureCoordinates = glm::vec2(
				attributes.texcoords[2 * idx2.texcoord_index + 0],
				1.0f - attributes.texcoords[2 * idx2.texcoord_index + 1]
			);

			vertex0.Normal = glm::vec3(
				attributes.normals[3 * idx0.normal_index + 0],
				attributes.normals[3 * idx0.normal_index + 1],
				attributes.normals[3 * idx0.normal_index + 2]
			);
			vertex1.Normal = glm::vec3(
				attributes.normals[3 * idx1.normal_index + 0],
				attributes.normals[3 * idx1.normal_index + 1],
				attributes.normals[3 * idx1.normal_index + 2]
			);
			vertex2.Normal = glm::vec3(
				attributes.normals[3 * idx2.normal_index + 0],
				attributes.normals[3 * idx2.normal_index + 1],
				attributes.normals[3 * idx2.normal_index + 2]
			);

			// Calculate edges of the triangle
			glm::vec3 edge1 = vertex1.Position - vertex0.Position;
			glm::vec3 edge2 = vertex2.Position - vertex0.Position;

			// Calculate the difference in UV coordinates
			glm::vec2 deltaUV1 = vertex1.TextureCoordinates - vertex0.TextureCoordinates;
			glm::vec2 deltaUV2 = vertex2.TextureCoordinates - vertex0.TextureCoordinates;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 tangent;
			tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			vertex0.Tangent += tangent;
			vertex1.Tangent += tangent;
			vertex2.Tangent += tangent;

			// Insert vertices into the uniqueVertices map and m_Vertices vector
			if (uniqueVertices.count(vertex0) == 0) {
				uniqueVertices[vertex0] = static_cast<uint32_t>(m_Vertices.size());
				m_Vertices.push_back(vertex0);
			}

			if (uniqueVertices.count(vertex1) == 0) {
				uniqueVertices[vertex1] = static_cast<uint32_t>(m_Vertices.size());
				m_Vertices.push_back(vertex1);
			}

			if (uniqueVertices.count(vertex2) == 0) {
				uniqueVertices[vertex2] = static_cast<uint32_t>(m_Vertices.size());
				m_Vertices.push_back(vertex2);
			}

			m_Indices.push_back(uniqueVertices[vertex0]);
			m_Indices.push_back(uniqueVertices[vertex1]);
			m_Indices.push_back(uniqueVertices[vertex2]);
		}
	}

	// Normalize the tangents
	for (auto& vertex : m_Vertices) {
		vertex.Tangent = glm::normalize(vertex.Tangent);
	}
}
