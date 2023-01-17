#pragma once

#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/pipeline/pipeline.hpp>

namespace vkx {
namespace pipeline {
struct GraphicsPipelineInformation {
	const std::string vertexFile{};
	const std::string fragmentFile{};
	const std::vector<vk::DescriptorSetLayoutBinding> bindings{};
	const std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
	const std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
	const std::vector<std::size_t> uniformSizes{};
	const std::vector<const Texture*> textures;
};

class GraphicsPipeline : public vkx::pipeline::VulkanPipeline {
	friend class CommandSubmitter;

private:
	vk::UniquePipeline pipeline{};
	vk::UniqueDescriptorPool descriptorPool{};
	std::vector<vk::DescriptorSet> descriptorSets{};
	std::vector<std::vector<UniformBuffer>> uniforms{};

public:
	GraphicsPipeline() = default;

	explicit GraphicsPipeline(vk::Device logicalDevice,
				  vk::RenderPass renderPass,
				  const vkx::VulkanAllocator& allocator,
				  const vkx::pipeline::GraphicsPipelineInformation& info);

	const std::vector<UniformBuffer>& getUniformByIndex(std::size_t i) const;
};
} // namespace pipeline
} // namespace vkx
