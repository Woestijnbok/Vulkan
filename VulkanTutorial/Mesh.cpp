#include "Mesh.h"

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

Mesh::Mesh(const std::vector<Vertex>& vertices) :
	m_Vertices{ vertices }
{

}

const std::vector<Vertex>& Mesh::GetVertices() const
{
	return m_Vertices;
}