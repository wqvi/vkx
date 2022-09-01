#pragma once

#include <vkx/renderer/core/allocator.hpp>
#include <vkx/renderer/core/device.hpp>

namespace vkx {
template <class T>
class UniformBuffer {
public:
	std::shared_ptr<Allocation<vk::Buffer>> resource{};
	T uniformObject{};

	UniformBuffer() = default;

	explicit UniformBuffer(const T& value, std::shared_ptr<vkx::Allocation<vk::Buffer>> resource)
	    : resource(resource), uniformObject(value) {}

	T* operator->() {
		return &uniformObject;
	}

	void mapMemory() const {
		std::memcpy(resource->allocationInfo.pMappedData, &uniformObject, resource->allocationInfo.size);
	}

	vk::DescriptorBufferInfo createDescriptorBufferInfo() const {
		return {resource->object, 0, sizeof(T)};
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