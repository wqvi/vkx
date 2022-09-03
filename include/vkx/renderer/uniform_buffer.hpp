#pragma once

#include <vkx/renderer/core/allocator.hpp>

namespace vkx {
class UniformBuffer {
private:
	std::shared_ptr<Allocation<vk::Buffer>> resource{};

public:
	UniformBuffer() = default;

	explicit UniformBuffer(std::shared_ptr<vkx::Allocation<vk::Buffer>> resource)
	    : resource(resource) {}

	template <class T>
	void mapMemory(const T& obj) const {
		std::memcpy(resource->allocationInfo.pMappedData, &obj, resource->allocationInfo.size);
	}

	vk::DescriptorBufferInfo createDescriptorBufferInfo() const {
		return {resource->object, 0, resource->allocationInfo.size};
	}
};

struct MVP {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct DirectionalLight {
	glm::vec3 position;		    // Position of the light in the world space
	alignas(16) glm::vec3 eyePosition;  // Position of the camera in the world space
	alignas(16) glm::vec4 ambientColor; // W is the intensity of the ambient light
	glm::vec3 diffuseColor;
	alignas(16) glm::vec3 specularColor;
	float constant;
	float linear;
	float quadratic;
};

struct Material {
	glm::vec3 specularColor;
	float shininess;
};
} // namespace vkx