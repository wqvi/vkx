#pragma once

namespace vkx {
namespace pipeline {
class VulkanPipeline {
protected:
	vk::Device logicalDevice{};

public:
	VulkanPipeline() = default;

private:
	[[nodiscard]] vk::UniqueShaderModule createShaderModule(const std::string& filename) const;
};
} // namespace pipeline
} // namespace vkx