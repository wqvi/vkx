#pragma once

namespace vkx {
struct Vertex {
	glm::vec2 pos{};
	glm::vec2 uv{};
	glm::vec2 normal{};

	Vertex() = default;

	Vertex(const glm::vec2& pos);

	explicit Vertex(const glm::vec2& pos,
			const glm::vec2& uv,
			const glm::vec2& normal);

	static auto getBindingDescription() noexcept {
		std::vector<vk::VertexInputBindingDescription> bindingDescriptions{
		    {0, sizeof(vkx::Vertex), vk::VertexInputRate::eVertex}};

		return bindingDescriptions;
	}

	static auto getAttributeDescriptions() noexcept {
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{
		    {0, 0, vk::Format::eR32G32Sfloat, offsetof(vkx::Vertex, pos)},
		    {1, 0, vk::Format::eR32G32Sfloat, offsetof(vkx::Vertex, uv)},
		    {2, 0, vk::Format::eR32G32Sfloat, offsetof(vkx::Vertex, normal)}};

		return attributeDescriptions;
	}
};
} // namespace vkx