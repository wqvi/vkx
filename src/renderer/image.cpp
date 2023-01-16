#include <vkx/renderer/image.hpp>

vkx::Image::Image(vk::Device logicalDevice, vk::UniqueImage&& image, vkx::alloc::UniqueVmaAllocation&& allocation)
    : logicalDevice(logicalDevice),
      resourceImage(std::move(image)),
      resourceAllocation(std::move(allocation)) {
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
	    *resourceImage,
	    vk::ImageViewType::e2D,
	    format,
	    {},
	    subresourceRange};

	return logicalDevice.createImageViewUnique(imageViewCreateInfo);
}