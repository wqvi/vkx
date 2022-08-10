#include "vkx/renderer/core/pipeline.hpp"
#include <memory>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/core/sync_objects.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

vkx::Allocator::Allocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance) {
	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.flags = 0;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.preferredLargeHeapBlockSize = 0;
	allocatorCreateInfo.pAllocationCallbacks = nullptr;
	allocatorCreateInfo.pDeviceMemoryCallbacks = nullptr;
	allocatorCreateInfo.pHeapSizeLimit = nullptr;
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
	allocatorCreateInfo.instance = instance;
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

std::shared_ptr<vkx::Allocation<vk::Image>> vkx::Allocator::allocateImage(std::uint32_t width, std::uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::Extent3D imageExtent(width, height, 1);

	const vk::ImageCreateInfo imageCreateInfo({}, vk::ImageType::e2D, format, imageExtent, 1, 1, vk::SampleCountFlagBits::e1, tiling, imageUsage, vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined);

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = flags;
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

	VkImage image = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateImage(allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &image, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	return std::make_shared<Allocation<vk::Image>>(vk::Image(image), allocation, allocationInfo, allocator);
}

std::shared_ptr<vkx::Allocation<vk::Buffer>> vkx::Allocator::allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo({}, size, bufferUsage, vk::SharingMode::eExclusive);

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = flags;
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	return std::make_shared<Allocation<vk::Buffer>>(vk::Buffer(buffer), allocation, allocationInfo, allocator);
}

VmaAllocationCreateInfo vkx::Allocator::createAllocationInfo(VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage, VmaPool pool) {
	return VmaAllocationCreateInfo{flags, memoryUsage, 0, 0, 0, pool, nullptr, {}};
}

vkx::Device::Device(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
    : instance(instance), physicalDevice(physicalDevice), properties(physicalDevice.getProperties()) {

	const QueueConfig queueConfig{physicalDevice, surface};
	if (!queueConfig.isComplete()) {
		throw std::runtime_error("Something went terribly wrong. Failure to find queue configuration.");
	}

	constexpr float queuePriority = 1.0f;
	const auto queueCreateInfos = queueConfig.createQueueInfos(queuePriority);

	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = true;

#ifdef DEBUG
	constexpr std::array layers{
	    "VK_LAYER_KHRONOS_validation"};
#elif RELEASE
	constexpr std::array<const char*, 0> layers{};
#endif

	constexpr std::array extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	vk::DeviceCreateInfo deviceCreateInfo{
	    {},
	    queueCreateInfos,
	    layers,
	    extensions,
	    &deviceFeatures};

	device = physicalDevice.createDeviceUnique(deviceCreateInfo);

	const vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, *queueConfig.graphicsIndex);

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
	const vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, size);

	const auto commandBuffers = device->allocateCommandBuffers(allocInfo);

	std::vector<DrawCommand> drawCommands;
	std::transform(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(drawCommands),
		       [&device = this->device](const auto& commandBuffer) -> DrawCommand {
			       return vkx::DrawCommand{device, commandBuffer};
		       });
	return drawCommands;
}

vk::Format vkx::Device::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const {
	for (const vk::Format format : candidates) {
		const auto formatProps = physicalDevice.getFormatProperties(format);

		bool isLinear = tiling == vk::ImageTiling::eLinear && (formatProps.linearTilingFeatures & features) == features;
		bool isOptimal = tiling == vk::ImageTiling::eOptimal && (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return vk::Format::eUndefined;
}

vk::UniqueImageView vkx::Device::createImageViewUnique(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const {
	const vk::ImageSubresourceRange subresourceRange(aspectFlags, 0, 1, 0, 1);

	const vk::ImageViewCreateInfo imageViewInfo({}, image, vk::ImageViewType::e2D, format, {}, subresourceRange);

	return device->createImageViewUnique(imageViewInfo);
}

void vkx::Device::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const {
	const SingleTimeCommand singleTimeCommand(*this, queues.graphics);

	const vk::BufferCopy copyRegion(0, 0, size);

	singleTimeCommand->copyBuffer(srcBuffer, dstBuffer, copyRegion);
}

void vkx::Device::copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const {
	const SingleTimeCommand singleTimeCommand{*this, queues.graphics};

	const vk::ImageSubresourceLayers subresourceLayer(vk::ImageAspectFlagBits::eColor, 0, 0, 1);

	const vk::Offset3D imageOffset(0, 0, 0);

	const vk::Extent3D imageExtent(width, height, 1);

	const vk::BufferImageCopy region(
	    0,
	    0,
	    0,
	    subresourceLayer,
	    imageOffset,
	    imageExtent);

	singleTimeCommand->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

void vkx::Device::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
	const SingleTimeCommand singleTimeCommand{*this, queues.graphics};

	const vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	vk::AccessFlags srcAccessMask;
	vk::AccessFlags dstAccessMask;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = vk::AccessFlagBits::eNone;
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else {
		throw std::invalid_argument("Unsupported layout transition.");
	}

	const vk::ImageMemoryBarrier barrier(
	    srcAccessMask,
	    dstAccessMask,
	    oldLayout,
	    newLayout,
	    VK_QUEUE_FAMILY_IGNORED,
	    VK_QUEUE_FAMILY_IGNORED,
	    image,
	    subresourceRange);

	singleTimeCommand->pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}

void vkx::Device::submit(const std::vector<vk::CommandBuffer>& commandBuffers, vk::Semaphore waitSemaphore, vk::Semaphore signalSemaphore, vk::Fence flightFence) const {
	constexpr std::array waitStage{vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)};
	const vk::SubmitInfo submitInfo(
	    1,
	    &waitSemaphore,
	    waitStage.data(),
	    static_cast<std::uint32_t>(commandBuffers.size()),
	    commandBuffers.data(),
	    1,
	    &signalSemaphore);

	queues.graphics.submit(submitInfo, flightFence);
}

vk::Result vkx::Device::present(vk::SwapchainKHR swapchain, std::uint32_t imageIndex, vk::Semaphore signalSemaphores) const {
	const vk::PresentInfoKHR presentInfo(
	    1,
	    &signalSemaphores,
	    1,
	    &swapchain,
	    &imageIndex);

	return queues.present.presentKHR(&presentInfo);
}

[[nodiscard]] vk::UniqueImageView vkx::Device::createTextureImageViewUnique(vk::Image image) const {
	return createImageViewUnique(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

[[nodiscard]] vk::UniqueSampler vkx::Device::createTextureSamplerUnique() const {
	const vk::SamplerCreateInfo samplerInfo(
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
	    false);

	return device->createSamplerUnique(samplerInfo);
}

std::shared_ptr<vkx::Allocator> vkx::Device::createAllocator() const {
	return std::make_shared<vkx::Allocator>(physicalDevice, *device, instance);
}

std::shared_ptr<vkx::Swapchain> vkx::Device::createSwapchain(SDL_Window* window, const std::shared_ptr<vkx::Allocator>& allocator) const {
	return std::make_shared<vkx::Swapchain>(*this, surface, window, allocator);
}

vk::UniqueRenderPass vkx::Device::createRenderPass(vk::Format format, vk::AttachmentLoadOp loadOp) const {
	const vk::AttachmentDescription colorAttachment(
	    {},
	    format,
	    vk::SampleCountFlagBits::e1,
	    loadOp,
	    vk::AttachmentStoreOp::eStore,
	    vk::AttachmentLoadOp::eDontCare,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::ImageLayout::eUndefined,
	    vk::ImageLayout::ePresentSrcKHR);

	const vk::AttachmentReference colorAttachmentRef(
	    0,
	    vk::ImageLayout::eColorAttachmentOptimal);

	const vk::AttachmentDescription depthAttachment(
	    {},
	    findDepthFormat(),
	    vk::SampleCountFlagBits::e1,
	    vk::AttachmentLoadOp::eClear,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::AttachmentLoadOp::eDontCare,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::ImageLayout::eUndefined,
	    vk::ImageLayout::eDepthStencilAttachmentOptimal);

	const vk::AttachmentReference depthAttachmentRef(
	    1,
	    vk::ImageLayout::eDepthStencilAttachmentOptimal);

	const vk::SubpassDescription subpass(
	    {},
	    vk::PipelineBindPoint::eGraphics,
	    {},
	    colorAttachmentRef,
	    {},
	    &depthAttachmentRef,
	    {});

	constexpr auto dependencyStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	constexpr auto dependencyAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	const vk::SubpassDependency dependency(
	    VK_SUBPASS_EXTERNAL,
	    0,
	    dependencyStageMask,
	    dependencyStageMask,
	    {},
	    dependencyAccessMask);

	const auto renderPassAttachments = {colorAttachment, depthAttachment};

	const vk::RenderPassCreateInfo renderPassInfo(
	    {},
	    renderPassAttachments,
	    subpass,
	    dependency);

	return device->createRenderPassUnique(renderPassInfo);
}

std::shared_ptr<vkx::GraphicsPipeline> vkx::Device::createGraphicsPipeline(const vk::Extent2D& extent, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout) const {
	return std::make_shared<vkx::GraphicsPipeline>(*this, extent, renderPass, descriptorSetLayout);
}