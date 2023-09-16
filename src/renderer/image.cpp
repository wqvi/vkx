#include <vkx/renderer/image.hpp>

vkx::Image::Image(vk::Device logicalDevice, VmaAllocator allocator, VkImage image, VmaAllocation allocation)
    : logicalDevice(logicalDevice), allocator(allocator), resourceImage(image), resourceAllocation(allocation) {}

vkx::Image::Image(vk::Device logicalDevice, VmaAllocator allocator, vk::Extent2D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage)
	: logicalDevice(logicalDevice), 
	allocator(allocator) {
	const vk::Extent3D imageExtent{extent.width, extent.height, 1};

	const vk::ImageCreateInfo imageCreateInfo{
	    {},
	    vk::ImageType::e2D,
	    format,
	    imageExtent,
	    1,
	    1,
	    vk::SampleCountFlagBits::e1,
	    tiling,
	    imageUsage,
	    vk::SharingMode::eExclusive};

	VmaAllocationCreateInfo allocationCreateInfo{
	    flags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	if (vmaCreateImage(allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &resourceImage, &resourceAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}
}

vkx::Image::operator vk::Image() const {
	return static_cast<vk::Image>(resourceImage);
}

void vkx::Image::destroy() const {
	vmaDestroyImage(allocator, resourceImage, resourceAllocation);
}

vk::ImageView vkx::Image::createView(vk::Format format, vk::ImageAspectFlags aspectFlags) const {
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

	return logicalDevice.createImageView(imageViewCreateInfo);
}
