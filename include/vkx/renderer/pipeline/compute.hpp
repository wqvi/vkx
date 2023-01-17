#pragma once

#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/pipeline/pipeline.hpp>

namespace vkx {
namespace pipeline {
struct ComputePipelineInformation {
	const std::string file{};
	const std::vector<vk::DescriptorSetLayoutBinding> bindings{};
	const std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
	const std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
	const std::vector<std::size_t> uniformSizes{};
	const std::vector<const Texture*> textures;
};

class ComputePipeline : public vkx::pipeline::VulkanPipeline {
private:
	vk::Device logicalDevice{};
	vk::UniqueDescriptorSetLayout descriptorLayout{};
	vk::UniquePipelineLayout pipelineLayout{};
	vk::UniquePipeline pipeline{};
	vk::DescriptorPool descriptorPool{};
	std::vector<vk::DescriptorSet> descriptorSets{};

public:
	ComputePipeline() = default;

	explicit ComputePipeline(vk::Device logicalDevice,
				 const vkx::VulkanAllocator& allocator,
				 const vkx::pipeline::ComputePipelineInformation& info);
};
} // namespace pipeline
} // namespace vkx