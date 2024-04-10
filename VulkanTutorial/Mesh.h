#ifndef MESH
#define MESH

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

struct Vertex final
{
	glm::vec2 Position;
	glm::vec3 Color;
	glm::vec2 TextureCoordinates;

	static VkVertexInputBindingDescription GetBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
};

class Mesh final
{
public:
	Mesh
	(
		VkPhysicalDevice physicalDevice, 
		VkDevice device, 
		VkCommandPool copyCommandPool, 
		VkQueue copyQueue, 
		const std::vector<Vertex>& vertices, 
		const std::vector<uint16_t>& indices
	);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	const std::vector<Vertex>& GetVertices() const;
	VkBuffer GetVertexBuffer() const;
	const std::vector<uint16_t> GetIndices() const;
	VkBuffer GetIndexBuffer() const;

private:
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkCommandPool m_CopyCommandPool;
	VkQueue m_CopyQueue;
	std::vector<Vertex> m_Vertices;
	VkBuffer m_VertexStagingBuffer;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexStagingBufferMemory;
	VkDeviceMemory m_VertexBufferMemory;
	std::vector<uint16_t> m_Indices;
	VkBuffer m_IndexStagingBuffer;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexStagingBufferMemory;
	VkDeviceMemory m_IndexBufferMemory;

	VkResult CreateVertexBuffer();
	VkResult CreateIndexBuffer();
};

#endif