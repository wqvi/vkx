#pragma once

namespace vkx {
class VulkanPipeline {
private:
	vk::Device logicalDevice{};

private:
	[[nodiscard]] vk::UniqueShaderModule createShaderModule() const;
};
} // namespace vkx