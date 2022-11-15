#pragma once

#include <vkx/renderer/core/allocator.hpp>

namespace vkx {
class UniformBuffer {
private:
	VmaAllocator allocator = nullptr;
	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo{};
	VkDescriptorBufferInfo info{};

public:
	UniformBuffer() = default;

	explicit UniformBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo)
	    : allocator(allocator),
	      buffer(buffer),
	      allocation(allocation),
	      allocationInfo(allocationInfo),
	      info{buffer, 0, allocationInfo.size} {}

	template <class T>
	void mapMemory(const T& obj) const {
		std::memcpy(allocationInfo.pMappedData, &obj, allocationInfo.size);
	}

	VkDescriptorBufferInfo createDescriptorBufferInfo() const {
		return {buffer, 0, allocationInfo.size};
	}

	const VkDescriptorBufferInfo* getInfo() const {
		return &info;
	}

	void destroy() const {
		vmaDestroyBuffer(allocator, buffer, allocation);
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
