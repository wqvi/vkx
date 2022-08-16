#pragma once

#include <cstddef>
#include <vulkan/vulkan_enums.hpp>
namespace vkx {
struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;

	static auto getBindingDescription() noexcept {
		std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};

		bindingDescriptions.push_back({0, sizeof(Vertex), vk::VertexInputRate::eVertex});

		return bindingDescriptions;
	}

	static auto getAttributeDescriptions() noexcept {
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)});
		attributeDescriptions.push_back({1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)});
		attributeDescriptions.push_back({2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)});

		return attributeDescriptions;
	}
};
} // namespace vkx