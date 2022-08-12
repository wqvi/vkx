#pragma once

#include <vkx/renderer/core/renderer_types.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/core/vertex.hpp>

namespace vkx {
class GraphicsPipeline {
public:
	vk::UniquePipelineLayout layout;
	vk::UniquePipeline pipeline;

	GraphicsPipeline() = default;

	GraphicsPipeline(vk::Device device, const vk::Extent2D& extent, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout);

private:
	static vk::UniquePipelineLayout createPipelineLayout(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout);

	static vk::UniqueShaderModule createShaderModule(vk::Device device, const std::string& filename);

	static vk::UniquePipeline createPipeline(vk::Device device, const vk::Extent2D& extent, vk::RenderPass renderPass, vk::PipelineLayout layout);
};
} // namespace vkx
