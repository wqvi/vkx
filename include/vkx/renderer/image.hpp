#pragma once

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

	explicit Image(vk::Device logicalDevice, VmaAllocator allocator, vk::Extent2D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage);

	explicit operator vk::Image() const;

	void destroy() const;

	vk::ImageView createView(vk::Format format, vk::ImageAspectFlags aspectFlags) const;
};
} // namespace vkx
