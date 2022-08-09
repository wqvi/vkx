#pragma once

#include "vkx/renderer/core/allocable.hpp"
#include <memory>
#include <vkx/renderer/core/device.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
template <class T>
class UniformBuffer {
public:
	std::shared_ptr<Allocation<vk::Buffer>> resource;

	UniformBuffer() = default;

	explicit UniformBuffer(const T& value, const std::shared_ptr<Allocator>& allocator) {
		resource = allocator->allocateBuffer(value, vk::BufferUsageFlagBits::eUniformBuffer);
		uniformObject = value;
	}

	T* operator->() {
		return &uniformObject;
	}

	void mapMemory() const {
		std::memcpy(resource->allocationInfo.pMappedData, &uniformObject, resource->allocationInfo.size);
	}

	void mapMemory(T const& data) const {
		std::memcpy(resource->allocationInfo.pMappedData, &data, resource->allocationInfo.size);
	}

	void setObject(T const& value) {
		uniformObject = value;
	}

	vk::WriteDescriptorSet createWriteDescriptorSet(const vk::DescriptorSet& descriptorSet, std::uint32_t dstBinding) const {
		// This is a type safe uniform buffer. There will only be one info per type of this uniform buffer
		static vk::DescriptorBufferInfo info{};

		info = vk::DescriptorBufferInfo{
		    resource->object, // buffer
		    0,		      // offset
		    sizeof(T)	      // range
		};

		static vk::WriteDescriptorSet set{};

		set = vk::WriteDescriptorSet{
		    descriptorSet,			// dstSet
		    dstBinding,				// dstBinding
		    0,					// dstArrayElement
		    1,					// descriptorCount
		    vk::DescriptorType::eUniformBuffer, // descriptorType
		    nullptr,				// pImageInfo
		    &info				// pBufferInfo
		};
		return set;
	}

private:
	T uniformObject;
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