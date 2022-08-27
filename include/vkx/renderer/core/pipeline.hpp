#pragma once

#include <vkx/renderer/core/vertex.hpp>

namespace vkx {
struct GraphicsPipelineInformation {
	std::string vertexFile;
	std::string fragmentFile;
	vk::RenderPass renderPass;
	vk::DescriptorSetLayout descriptorSetLayout;
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
};

class GraphicsPipeline {
public:
	vk::UniquePipelineLayout layout;
	vk::UniquePipeline pipeline;

	GraphicsPipeline() = default;

	explicit GraphicsPipeline(vk::Device device, const GraphicsPipelineInformation& info);

private:
	static vk::UniquePipelineLayout createPipelineLayout(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout);

	static vk::UniqueShaderModule createShaderModule(vk::Device device, const std::string& filename);

	static vk::UniquePipeline createPipeline(vk::Device device, const GraphicsPipelineInformation& info, vk::PipelineLayout pipelineLayout);
};
} // namespace vkx
