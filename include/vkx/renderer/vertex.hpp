#pragma once

namespace vkx {
struct Vertex {
	glm::vec2 pos{};
	glm::vec2 uv{};

	Vertex() = default;

	Vertex(const glm::vec2& pos);

	explicit Vertex(const glm::vec2& pos,
			const glm::vec2& uv);

	static auto getBindingDescription() noexcept {
		std::vector<vk::VertexInputBindingDescription> bindingDescriptions{
		    {0, sizeof(Vertex), vk::VertexInputRate::eVertex}};

		return bindingDescriptions;
	}

	static auto getAttributeDescriptions() noexcept {
		constexpr auto format = vk::Format::eR32G32Sfloat;

		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{
		    {0, 0, format, offsetof(Vertex, pos)},
		    {1, 0, format, offsetof(Vertex, uv)}};

		return attributeDescriptions;
	}
};
} // namespace vkx