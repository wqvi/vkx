#pragma once

namespace vkx {
struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;

	static auto getBindingDescription() noexcept {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};

		bindingDescriptions.push_back({0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX});

		return bindingDescriptions;
	}

	static auto getAttributeDescriptions() noexcept {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)});
		attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
		attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});

		

		return attributeDescriptions;
	}
};
} // namespace vkx