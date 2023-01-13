#pragma once

#include <vkx/renderer/types.hpp>
#include <vkx/renderer/renderer.hpp>

namespace vkx {
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

class Buffer {
private:
	vk::UniqueBuffer buffer;
	vkx::UniqueVulkanAllocation allocation;
	VmaAllocationInfo allocationInfo{};

public:
	Buffer() = default;

	explicit Buffer(vk::UniqueBuffer&& buffer, vkx::UniqueVulkanAllocation&& allocation, VmaAllocationInfo&& allocationInfo);

	explicit operator vk::Buffer() const;

	template <class T>
	void mapMemory(const T* data) const {
		std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
	}

	template <class T>
	void mapMemory(const T* data, std::size_t memoryOffset) const {
		T* ptr = reinterpret_cast<T*>(allocationInfo.pMappedData) + memoryOffset;
		const auto size = allocationInfo.size - memoryOffset;
		std::memcpy(ptr, data, size);
	}

	std::size_t size() const;
};

class UniformBuffer {
private:
	vk::DescriptorBufferInfo info{};
	vkx::Buffer buffer{};

public:
	UniformBuffer() = default;

	explicit UniformBuffer(vkx::Buffer&& buffer);

	template <class T>
	inline void mapMemory(const T& obj) const {
		buffer.mapMemory(&obj);
	}

	const vk::DescriptorBufferInfo* getInfo() const noexcept;
};
} // namespace vkx
