#pragma once

namespace vkx {
namespace pipeline {
class VulkanPipeline {
private:
	vk::Device logicalDevice{};

private:
	[[nodiscard]] vk::UniqueShaderModule createShaderModule(const std::string& filename) const;
};
} // namespace pipeline
} // namespace vkx