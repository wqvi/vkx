#include <vkx/renderer/image.hpp>

vkx::Image::Image(vk::Device logicalDevice, VmaAllocator allocator, VkImage image, VmaAllocation allocation)
    : logicalDevice(logicalDevice), allocator(allocator), resourceImage(image), resourceAllocation(allocation) {}

vkx::Image::operator vk::Image() const {
	return static_cast<vk::Image>(resourceImage);
}

void vkx::Image::destroy() const {
	vmaDestroyImage(allocator, resourceImage, resourceAllocation);
}

vk::UniqueImageView vkx::Image::createView(vk::Format format, vk::ImageAspectFlags aspectFlags) const {
	const vk::ImageSubresourceRange subresourceRange{
	    aspectFlags,
	    0,
	    1,
	    0,
	    1};

	const vk::ImageViewCreateInfo imageViewCreateInfo{
	    {},
	    resourceImage,
	    vk::ImageViewType::e2D,
	    format,
	    {},
	    subresourceRange};

	return logicalDevice.createImageViewUnique(imageViewCreateInfo);
}
