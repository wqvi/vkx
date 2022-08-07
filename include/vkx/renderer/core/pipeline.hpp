#pragma once

#include "device.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
class GraphicsPipeline {
public:
  vk::UniquePipelineLayout layout;
	vk::UniquePipeline pipeline;

	GraphicsPipeline() = default;

	GraphicsPipeline(const Device& device, const vk::Extent2D& extent, const vk::UniqueRenderPass& renderPass, const vk::UniqueDescriptorSetLayout& descriptorSetLayout);

private:
  static vk::UniquePipelineLayout createPipelineLayout(const Device& device, const vk::UniqueDescriptorSetLayout& descriptorSetLayout);

	static vk::UniqueShaderModule createShaderModule(const Device& device, const std::string& filename);
};
} // namespace vkx
