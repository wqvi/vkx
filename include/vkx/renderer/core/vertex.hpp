#pragma once

namespace vkx {
struct Vertex {
	glm::vec2 pos;
	glm::vec2 uv;
	glm::vec2 normal;

	Vertex() = default;

	explicit Vertex(const glm::vec2& pos, const glm::vec2& uv, const glm::vec2& normal);

	static auto getBindingDescription() noexcept {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions{
		    {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};

		return bindingDescriptions;
	}

	static auto getAttributeDescriptions() noexcept {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{
		    {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos)},
		    {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)},
		    {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, normal)}};

		return attributeDescriptions;
	}
};
} // namespace vkx