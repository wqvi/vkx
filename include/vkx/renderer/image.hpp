#pragma once

#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Image {
private:
	vk::Device logicalDevice{};
	vk::UniqueImage resourceImage{};
	vkx::UniqueVulkanAllocation resourceAllocation{};

public:
	Image() = default;

	explicit Image(vk::Device logicalDevice, vk::UniqueImage&& image, UniqueVulkanAllocation&& allocation);

	vk::UniqueImageView createView(vk::Format format, vk::ImageAspectFlags aspectFlags) const;
};
} // namespace vkx