#pragma once

namespace vkx {
namespace pipeline {
class VulkanPipeline {
private:
	vk::Device logicalDevice{};
	vk::UniqueDescriptorSetLayout descriptorLayout{};
	vk::UniquePipelineLayout pipelineLayout{};
	vk::UniquePipeline pipeline{};

protected:
	VulkanPipeline() = default;

	explicit VulkanPipeline(vk::Device logicalDevice,
				const std::vector<vk::DescriptorSetLayoutBinding>& bindings);

	[[nodiscard]] vk::UniqueShaderModule createShaderModule(const std::string& filename) const;
};
} // namespace pipeline
} // namespace vkx
