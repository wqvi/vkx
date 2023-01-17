#pragma once

namespace vkx {
namespace pipeline {
class VulkanPipeline {
protected:
	vk::Device logicalDevice{};
	vk::UniqueDescriptorSetLayout descriptorLayout{};
	vk::UniquePipelineLayout pipelineLayout{};

	VulkanPipeline() = default;

	explicit VulkanPipeline(vk::Device logicalDevice,
				const std::vector<vk::DescriptorSetLayoutBinding>& bindings);

	[[nodiscard]] vk::UniqueShaderModule createShaderModule(const std::string& filename) const;
};
} // namespace pipeline
} // namespace vkx