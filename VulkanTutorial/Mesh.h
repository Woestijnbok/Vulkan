#ifndef MESH
#define MESH

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <filesystem>

struct Vertex final
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec2 TextureCoordinates;

	bool operator==(const Vertex& other) const;

	static VkVertexInputBindingDescription GetBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
};

namespace std 
{
	template <> struct hash<Vertex> 
	{
		size_t operator()(const Vertex& vertex) const 
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

			return hashValue;
		}

	private:
		// Helper function to combine hash values
		template <typename T>
		void hash_combine(size_t& seed, const T& v) const 
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}

class Mesh final
{
public:
	Mesh
	(
		VkPhysicalDevice physicalDevice, 
		VkDevice device, 
		VkCommandPool copyCommandPool, 
		VkQueue copyQueue, 
		const std::filesystem::path& path
	);
	Mesh
	(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkCommandPool copyCommandPool,
		VkQueue copyQueue,
		const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices
	);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	const std::vector<Vertex>& GetVertices() const;
	VkBuffer GetVertexBuffer() const;
	const std::vector<uint32_t> GetIndices() const;
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
	std::vector<uint32_t> m_Indices;
	VkBuffer m_IndexStagingBuffer;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexStagingBufferMemory;
	VkDeviceMemory m_IndexBufferMemory;

	void LoadMesh(const std::filesystem::path& path);
	VkResult CreateVertexBuffer();
	VkResult CreateIndexBuffer();
};

#endif