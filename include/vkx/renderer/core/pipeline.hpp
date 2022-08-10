#pragma once

#include "device.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
class GraphicsPipeline {
public:
	vk::UniquePipelineLayout layout;
	vk::UniquePipeline pipeline;

	GraphicsPipeline() = default;

	GraphicsPipeline(const vkx::Device& device, const vk::Extent2D& extent, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout);

private:
	static vk::UniquePipelineLayout createPipelineLayout(const vkx::Device& device, vk::DescriptorSetLayout descriptorSetLayout);

	static vk::UniqueShaderModule createShaderModule(const vkx::Device& device, const std::string& filename);

	static vk::UniquePipeline createPipeline(const vkx::Device& device, const vk::Extent2D& extent, vk::RenderPass renderPass, vk::PipelineLayout layout);
};
} // namespace vkx
