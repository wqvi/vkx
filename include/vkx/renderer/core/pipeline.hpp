#pragma once

#include <vkx/renderer/core/vertex.hpp>

namespace vkx {
struct GraphicsPipelineInformation {
	std::string vertexFile;
	std::string fragmentFile;
	vk::Device device;
	vk::Extent2D extent;
	vk::RenderPass renderPass;
	vk::DescriptorSetLayout descriptorSetLayout;
};

class GraphicsPipeline {
public:
	vk::UniquePipelineLayout layout;
	vk::UniquePipeline pipeline;

	GraphicsPipeline() = default;

	explicit GraphicsPipeline(const GraphicsPipelineInformation& info);

private:
	static vk::UniquePipelineLayout createPipelineLayout(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout);

	static vk::UniqueShaderModule createShaderModule(vk::Device device, const std::string& filename);

	static vk::UniquePipeline createPipeline(const GraphicsPipelineInformation& info, vk::PipelineLayout pipelineLayout);
};
} // namespace vkx
