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

	static VkVertexInputBindingDescription GetBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
};

class Mesh final
{
public:
	Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool copyCommandPool, VkQueue copyQueue, const std::vector<Vertex>& vertices);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	const std::vector<Vertex>& GetVertices() const;
	VkBuffer GetVertexBuffer() const;

private:
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkCommandPool m_CopyCommandPool;
	VkQueue m_CopyQueue;
	std::vector<Vertex> m_Vertices;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;

	VkResult CreateVertexBuffer();
};

#endif