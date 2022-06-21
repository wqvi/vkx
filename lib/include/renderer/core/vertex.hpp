#pragma once

namespace vkx
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;

		static constexpr std::array<vk::VertexInputBindingDescription, 1> getBindingDescription();

		static constexpr std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
	};
}

#include "vertex.inl"