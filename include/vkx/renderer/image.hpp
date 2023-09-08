#pragma once

#include <vkx/renderer/renderer.hpp>

namespace vkx {
class Image {
private:
	vk::Device logicalDevice{};
	VmaAllocator allocator;
	VkImage resourceImage{};
	VmaAllocation resourceAllocation{};

public:
	Image() = default;

	explicit Image(vk::Device logicalDevice, VmaAllocator allocator, VkImage image, VmaAllocation allocation);

	explicit operator vk::Image() const;

	void destroy() const;

	vk::UniqueImageView createView(vk::Format format, vk::ImageAspectFlags aspectFlags) const;
};
} // namespace vkx
