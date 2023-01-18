#pragma once

#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/pipeline/pipeline.hpp>

namespace vkx {
namespace pipeline {
struct ComputePipelineInformation {
	const std::string computeFile{};
	const std::vector<vk::DescriptorSetLayoutBinding> bindings{};
	const std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
	const std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
	const std::vector<std::size_t> uniformSizes{};
	const std::vector<const Texture*> textures;
};

class ComputePipeline : public vkx::pipeline::VulkanPipeline {
private:
	vk::UniqueDescriptorPool descriptorPool{};
	std::vector<vk::DescriptorSet> descriptorSets{};

public:
	ComputePipeline() = default;

	explicit ComputePipeline(vk::Device logicalDevice,
				 const vkx::pipeline::ComputePipelineInformation& info);
};
} // namespace pipeline
} // namespace vkx