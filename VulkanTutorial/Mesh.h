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
	Mesh(const std::vector<Vertex>& vertices);
	~Mesh() = default;

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	const std::vector<Vertex>& GetVertices() const;

private:
	std::vector<Vertex> m_Vertices;
};

#endif