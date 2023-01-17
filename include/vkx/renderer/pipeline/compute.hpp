#pragma once

namespace vkx {
class ComputePipeline {
private:
	vk::Device logicalDevice{};
	vk::UniqueDescriptorSetLayout descriptorLayout{};
	vk::UniquePipelineLayout pipelineLayout{};
	vk::UniquePipeline pipeline{};
	vk::DescriptorPool descriptorPool{};
	std::vector<vk::DescriptorSet> descriptorSets{};

public:
	ComputePipeline() = default;


};
} // namespace vkx