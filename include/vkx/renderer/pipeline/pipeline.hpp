#pragma once

namespace vkx {
namespace pipeline {
class VulkanPipeline {
protected:
	vk::Device logicalDevice{};

	VulkanPipeline() = default;

	explicit VulkanPipeline(vk::Device logicalDevice);

	[[nodiscard]] vk::UniqueShaderModule createShaderModule(const std::string& filename) const;
};
} // namespace pipeline
} // namespace vkx