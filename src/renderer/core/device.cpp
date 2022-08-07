#include <functional>
#include <stdexcept>
#include <vkx/renderer/core/device.hpp>

#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/vkx_exceptions.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <iostream>

vkx::Allocator::Allocator() {
	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.flags = 0;
	allocatorCreateInfo.physicalDevice = nullptr;
	allocatorCreateInfo.device = nullptr;
	allocatorCreateInfo.preferredLargeHeapBlockSize = 0;
	allocatorCreateInfo.pAllocationCallbacks = nullptr;
	allocatorCreateInfo.pDeviceMemoryCallbacks = nullptr;
	allocatorCreateInfo.pHeapSizeLimit = nullptr;
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
	allocatorCreateInfo.instance = nullptr;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
#if VMA_EXTERNAL_MEMORY
	allocatorCreateInfo.pTypeExternalMemoryHandleTypes = nullptr;
#endif

	if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan memory allocator.");
	}
}

vkx::Allocator::~Allocator() {
	vmaDestroyAllocator(allocator);
}

VmaAllocator vkx::Allocator::getAllocator() const noexcept {
	return allocator;
}

vkx::Allocation<vk::Image> vkx::Allocator::allocateImage(std::uint32_t width, std::uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage) const {
	vk::Extent3D imageExtent(width, height, 1);

	vk::ImageCreateInfo imageCreateInfo({}, vk::ImageType::e2D, format, imageExtent, 1, 1, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined);

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = 0;
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

	VkImage image = nullptr;
	VmaAllocation allocation = nullptr;
	if (vmaCreateImage(allocator, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	return {vk::Image(image), allocation, allocator};
}

vkx::Allocation<vk::Buffer> vkx::Allocator::allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage) const {
	vk::BufferCreateInfo bufferCreateInfo({},size,usage,vk::SharingMode::eExclusive);

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = 0;
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	if (vmaCreateBuffer(allocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	return {vk::Buffer(buffer), allocation, allocator};
}

vkx::Device::Device(const vk::UniqueInstance& instance,
		    const vk::PhysicalDevice& physicalDevice,
		    const vk::UniqueSurfaceKHR& surface)
    : physicalDevice(physicalDevice), properties(physicalDevice.getProperties()) {

	QueueConfig queueConfig{physicalDevice, surface};
	if (!queueConfig.isComplete()) {
		throw vkx::VulkanError("Something went terribly wrong. Failure to find queue configuration.");
	}

	float queuePriority = 1.0f;
	auto queueCreateInfos = queueConfig.createQueueInfos(queuePriority);

	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = true;

	std::vector<const char*> layers{
#ifdef DEBUG
	    "VK_LAYER_KHRONOS_validation",
#endif
	};

	std::vector<const char*> extensions{
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	vk::DeviceCreateInfo deviceCreateInfo{
	    {},
	    queueCreateInfos,
	    layers,
	    extensions,
	    &deviceFeatures};

	device = physicalDevice.createDeviceUnique(deviceCreateInfo);

	vk::CommandPoolCreateInfo commandPoolInfo{
	    vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
	    queueConfig.graphicsIndex};

	commandPool = device->createCommandPoolUnique(commandPoolInfo);

	queues = Queues(*this, queueConfig);
}

vkx::Device::operator const vk::PhysicalDevice&() const {
	return physicalDevice;
}

vkx::Device::operator const vk::Device&() const {
	return device.get();
}

vkx::Device::operator const vk::CommandPool&() const {
	return commandPool.get();
}

vkx::Device::operator const vk::UniqueCommandPool&() const {
	return commandPool;
}

const vk::Device& vkx::Device::operator*() const {
	return *device;
}

const vk::Device* vkx::Device::operator->() const {
	return &*device;
}

std::vector<vkx::DrawCommand> vkx::Device::createDrawCommands(std::uint32_t size) const {
	vk::CommandBufferAllocateInfo allocInfo{
	    *commandPool,
	    vk::CommandBufferLevel::ePrimary,
	    size};

	auto commandBuffers = device->allocateCommandBuffers(allocInfo);

	std::vector<DrawCommand> drawCommands;
	const vk::UniqueDevice& tmp = device;
	std::transform(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(drawCommands),
		       [&tmp](const vk::CommandBuffer& commandBuffer) -> DrawCommand {
			       return vkx::DrawCommand{tmp, commandBuffer};
		       });
	return drawCommands;
}

std::uint32_t vkx::Device::findMemoryType(std::uint32_t typeFilter, const vk::MemoryPropertyFlags& flags) const {
	auto memProperties = physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
			return i;
		}
	}

	throw vkx::VulkanError("Failure to find suitable physical device memory type.");
}

vk::Format vkx::Device::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
					    const vk::FormatFeatureFlags& features) const {
	for (vk::Format format : candidates) {
		auto formatProps = physicalDevice.getFormatProperties(format);

		bool isLinear = tiling == vk::ImageTiling::eLinear &&
				(formatProps.linearTilingFeatures & features) == features;
		bool isOptimal = tiling == vk::ImageTiling::eOptimal &&
				 (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return vk::Format::eUndefined;
}

vk::Format vkx::Device::findDepthFormat() const {
	return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
				   vk::ImageTiling::eOptimal,
				   vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::UniqueImage
vkx::Device::createImageUnique(std::uint32_t width, std::uint32_t height, vk::Format format, vk::ImageTiling tiling,
			       const vk::ImageUsageFlags& usage) const {
	vk::Extent3D extent{
	    width,
	    height,
	    1};

	vk::ImageCreateInfo imageInfo{
	    {},
	    vk::ImageType::e2D,
	    format,
	    extent,
	    1,
	    1,
	    vk::SampleCountFlagBits::e1,
	    tiling,
	    usage,
	    vk::SharingMode::eExclusive,
	    {},
	    vk::ImageLayout::eUndefined};

	return device->createImageUnique(imageInfo);
}

vk::UniqueBuffer vkx::Device::createBufferUnique(vk::DeviceSize size, const vk::BufferUsageFlags& usage) const {
	vk::BufferCreateInfo bufferInfo{
	    {},
	    size,
	    usage,
	    vk::SharingMode::eExclusive};

	return device->createBufferUnique(bufferInfo);
}

vk::UniqueDeviceMemory
vkx::Device::allocateMemoryUnique(const vk::UniqueBuffer& buffer, const vk::MemoryPropertyFlags& flags) const {
	auto memReqs = device->getBufferMemoryRequirements(*buffer);

	vk::MemoryAllocateInfo allocInfo{
	    memReqs.size,
	    findMemoryType(memReqs.memoryTypeBits, flags)};

	auto memory = device->allocateMemoryUnique(allocInfo);

	device->bindBufferMemory(*buffer, *memory, 0);

	return memory;
}

vk::UniqueDeviceMemory
vkx::Device::allocateMemoryUnique(const vk::UniqueImage& image, const vk::MemoryPropertyFlags& flags) const {
	auto memReqs = device->getImageMemoryRequirements(*image);

	vk::MemoryAllocateInfo allocInfo{
	    memReqs.size,
	    findMemoryType(memReqs.memoryTypeBits, flags)};

	auto memory = device->allocateMemoryUnique(allocInfo);

	device->bindImageMemory(*image, *memory, 0);

	return memory;
}

vk::UniqueImageView vkx::Device::createImageViewUnique(const vk::Image& image, vk::Format format,
						       const vk::ImageAspectFlags& aspectFlags) const {
	vk::ImageSubresourceRange subresourceRange{
	    aspectFlags,
	    0,
	    1,
	    0,
	    1};

	vk::ImageViewCreateInfo imageViewInfo{
	    {},
	    image,
	    vk::ImageViewType::e2D,
	    format,
	    {},
	    subresourceRange};

	return device->createImageViewUnique(imageViewInfo);
}

void vkx::Device::copyBuffer(
    const vk::Buffer& srcBuffer,
    const vk::Buffer& dstBuffer,
    const vk::DeviceSize& size) const {
	SingleTimeCommand singleTimeCommand{*this, queues.graphics};

	vk::BufferCopy copyRegion{};
	copyRegion.size = size;
	singleTimeCommand->copyBuffer(srcBuffer, dstBuffer, copyRegion);
}

void vkx::Device::copyBufferToImage(
    const vk::Buffer& buffer,
    const vk::Image& image,
    std::uint32_t width,
    std::uint32_t height) const {
	SingleTimeCommand singleTimeCommand{*this, queues.graphics};

	vk::ImageSubresourceLayers subresourceLayer{
	    vk::ImageAspectFlagBits::eColor,
	    0,
	    0,
	    1};

	vk::Offset3D imageOffset{
	    0,
	    0,
	    0};
	vk::Extent3D imageExtent{
	    width,
	    height,
	    1};

	vk::BufferImageCopy region{
	    0,
	    0,
	    0,
	    subresourceLayer,
	    imageOffset,
	    imageExtent};

	singleTimeCommand->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

void vkx::Device::transitionImageLayout(
    const vk::Image& image,
    const vk::ImageLayout& oldLayout,
    const vk::ImageLayout& newLayout) const {
	SingleTimeCommand singleTimeCommand{*this, queues.graphics};

	vk::ImageSubresourceRange subresourceRange{
	    vk::ImageAspectFlagBits::eColor,
	    0,
	    1,
	    0,
	    1};

	vk::AccessFlags srcAccessMask;
	vk::AccessFlags dstAccessMask;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = {};
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
		   newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else {
		throw std::invalid_argument("Unsupported layout transition.");
	}

	vk::ImageMemoryBarrier barrier{
	    srcAccessMask,
	    dstAccessMask,
	    oldLayout,
	    newLayout,
	    VK_QUEUE_FAMILY_IGNORED,
	    VK_QUEUE_FAMILY_IGNORED,
	    image,
	    subresourceRange};

	singleTimeCommand->pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}

void vkx::Device::submit(const std::vector<vk::CommandBuffer>& commandBuffers, const vk::Semaphore& waitSemaphore,
			 const vk::Semaphore& signalSemaphore, const vk::Fence& flightFence) const {
	std::array<vk::PipelineStageFlags, 1> waitStage{
	    vk::PipelineStageFlagBits::eColorAttachmentOutput};
	vk::SubmitInfo submitInfo{
	    1,
	    &waitSemaphore,
	    waitStage.data(),
	    static_cast<std::uint32_t>(commandBuffers.size()),
	    commandBuffers.data(),
	    1,
	    &signalSemaphore};

	queues.graphics.submit(submitInfo, flightFence);
}

vk::Result vkx::Device::present(const vk::SwapchainKHR& swapchain, std::uint32_t imageIndex,
				const vk::Semaphore& signalSemaphores) const {
	vk::PresentInfoKHR presentInfo{
	    1,
	    &signalSemaphores,
	    1,
	    &swapchain,
	    &imageIndex};

	return queues.present.presentKHR(&presentInfo);
}

[[nodiscard]] vk::UniqueImageView vkx::Device::createTextureImageViewUnique(const vk::Image& image) const {
	return createImageViewUnique(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

[[nodiscard]] vk::UniqueSampler vkx::Device::createTextureSamplerUnique() const {
	vk::SamplerCreateInfo samplerInfo{
	    {},
	    vk::Filter::eLinear,
	    vk::Filter::eLinear,
	    vk::SamplerMipmapMode::eLinear,
	    vk::SamplerAddressMode::eRepeat,
	    vk::SamplerAddressMode::eRepeat,
	    vk::SamplerAddressMode::eRepeat,
	    {},
	    true,
	    properties.limits.maxSamplerAnisotropy,
	    false,
	    vk::CompareOp::eAlways,
	    {},
	    {},
	    vk::BorderColor::eIntOpaqueBlack,
	    false};

	return device->createSamplerUnique(samplerInfo);
}

void vkx::Device::submit(
    const std::vector<DrawCommand>& drawCommands,
    const SyncObjects& syncObjects) const {
	std::array<vk::PipelineStageFlags, 1> waitStage{
	    vk::PipelineStageFlagBits::eColorAttachmentOutput};

	std::vector<vk::CommandBuffer> commands;
	std::transform(drawCommands.begin(), drawCommands.end(), std::back_inserter(commands), [](const vkx::DrawCommand& drawCommand) {
		return static_cast<vk::CommandBuffer>(drawCommand);
	});

	vk::SubmitInfo submitInfo{
	    1,
	    &*syncObjects.imageAvailableSemaphore,
	    waitStage.data(),
	    static_cast<std::uint32_t>(commands.size()),
	    commands.data(),
	    1,
	    &*syncObjects.renderFinishedSemaphore};

	queues.graphics.submit(submitInfo, *syncObjects.inFlightFence);
}
