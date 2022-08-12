#include "vkx/renderer/core/pipeline.hpp"
#include "vkx/renderer/core/queue_config.hpp"
#include "vkx/renderer/core/renderer_types.hpp"
#include <SDL2/SDL_log.h>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/core/sync_objects.hpp>
#include <vulkan/vulkan_enums.hpp>
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
    : instance(instance), surface(surface), physicalDevice(physicalDevice), maxSamplerAnisotropy(physicalDevice.getProperties().limits.maxSamplerAnisotropy) {

	if (!static_cast<bool>(instance)) {
		throw std::invalid_argument("Instance must be a valid handle.");
	}

	if (!static_cast<bool>(physicalDevice)) {
		throw std::invalid_argument("Physical device must be a valid handle.");
	}

	if (!static_cast<bool>(surface)) {
		throw std::invalid_argument("Surface must be a valid handle.");
	}

	const QueueConfig queueConfig{physicalDevice, surface};
	if (!queueConfig.isComplete()) {
		throw std::runtime_error("Something went terribly wrong. Failure to find queue configuration.");
	}

	constexpr float queuePriority = 1.0f;
	const auto queueCreateInfos = queueConfig.createQueueInfos(queuePriority);

	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = true;

#ifdef DEBUG
	constexpr std::array layers{"VK_LAYER_KHRONOS_validation"};
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

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully created renderer device.");
}

vkx::Device::operator const vk::PhysicalDevice&() const {
	return physicalDevice;
}

vkx::Device::operator const vk::Device&() const {
	return device.get();
}

const vk::Device& vkx::Device::operator*() const {
	return *device;
}

const vk::Device* vkx::Device::operator->() const {
	return &*device;
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

vk::UniqueImageView vkx::Device::createTextureImageViewUnique(vk::Image image) const {
	return createImageViewUnique(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

vk::UniqueSampler vkx::Device::createTextureSamplerUnique() const {
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
	    maxSamplerAnisotropy,
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
	vk::ImageLayout colorLayout = vk::ImageLayout::eUndefined;
	if (loadOp == vk::AttachmentLoadOp::eLoad) {
		colorLayout = vk::ImageLayout::eColorAttachmentOptimal;
	}

	const vk::AttachmentDescription colorAttachment(
	    {},
	    format,
	    vk::SampleCountFlagBits::e1,
	    loadOp,
	    vk::AttachmentStoreOp::eStore,
	    vk::AttachmentLoadOp::eDontCare,
	    vk::AttachmentStoreOp::eDontCare,
	    colorLayout,
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
	return std::make_shared<vkx::GraphicsPipeline>(*device, extent, renderPass, descriptorSetLayout);
}

std::shared_ptr<vkx::CommandSubmitter> vkx::Device::createCommandSubmitter() const {
	return std::make_shared<vkx::CommandSubmitter>(physicalDevice, *device, surface);
}

vkx::CommandSubmitter::CommandSubmitter(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface)
    : device(device) {
	const vkx::QueueConfig queueConfig(physicalDevice, surface);

	graphicsQueue = device.getQueue(*queueConfig.graphicsIndex, 0);
	presentQueue = device.getQueue(*queueConfig.presentIndex, 0);

	const vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, *queueConfig.graphicsIndex);

	commandPool = device.createCommandPoolUnique(commandPoolInfo);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully created renderer command submitter.");
}

void vkx::CommandSubmitter::submitImmediately(const std::function<void(vk::CommandBuffer)>& command) const {
	const vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1);

	auto commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

	constexpr vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {}, nullptr);

	commandBuffer.begin(beginInfo);

	command(commandBuffer);

	commandBuffer.end();

	const vk::SubmitInfo submitInfo({}, {}, commandBuffer, {});

	static_cast<void>(graphicsQueue.submit(1, &submitInfo, {}));
	graphicsQueue.waitIdle();

	device.freeCommandBuffers(*commandPool, commandBuffer);
}

void vkx::CommandSubmitter::copyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) const {
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

	submitImmediately([&buffer, &image, &region](vk::CommandBuffer commandBuffer) {
		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	});
}

void vkx::CommandSubmitter::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {
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

	submitImmediately([&sourceStage, &destinationStage, &barrier](vk::CommandBuffer commandBuffer) {
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
	});
}

std::vector<vk::CommandBuffer> vkx::CommandSubmitter::allocateDrawCommands(std::uint32_t amount) const {
	const vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, amount * MAX_FRAMES_IN_FLIGHT);

	return device.allocateCommandBuffers(allocInfo);
}

void vkx::CommandSubmitter::recordDrawCommands(iter begin, iter end, const DrawInfo& drawInfo) const {
	for (; begin != end; begin++) {
		const auto commandBuffer = *begin;

		commandBuffer.reset({});
		const vk::CommandBufferBeginInfo beginInfo{};
		static_cast<void>(commandBuffer.begin(beginInfo));

		constexpr std::array clearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
		constexpr vk::ClearColorValue clearColor(clearColorValue);
		constexpr vk::ClearDepthStencilValue clearDepthStencil(1.0f, 0);

		constexpr std::array<vk::ClearValue, 2> clearValues{clearColor, clearDepthStencil};

		const vk::Rect2D renderArea({0, 0}, drawInfo.extent);

		const vk::RenderPassBeginInfo renderPassInfo(drawInfo.renderPass, drawInfo.framebuffer, renderArea, clearValues);

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, drawInfo.graphicsPipeline);

		commandBuffer.bindVertexBuffers(0, drawInfo.vertexBuffer, {0});

		commandBuffer.bindIndexBuffer(drawInfo.indexBuffer, 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, drawInfo.graphicsPipelineLayout, 0, drawInfo.descriptorSet, {});

		commandBuffer.drawIndexed(drawInfo.indexCount, 1, 0, 0, 0);

		commandBuffer.endRenderPass();

		commandBuffer.end();
	}
}

void vkx::CommandSubmitter::submitDrawCommands(iter begin, iter end, const SyncObjects& syncObjects) const {
	constexpr std::array waitStage{vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)};
	const vk::SubmitInfo submitInfo(
	    1,
	    &*syncObjects.imageAvailableSemaphore,
	    waitStage.data(),
	    static_cast<std::uint32_t>(std::distance(begin, end)),
	    &*begin,
	    1,
	    &*syncObjects.renderFinishedSemaphore);

	graphicsQueue.submit(submitInfo, *syncObjects.inFlightFence);
}

vk::Result vkx::CommandSubmitter::presentToSwapchain(vk::SwapchainKHR swapchain, std::uint32_t imageIndex, const SyncObjects& syncObjects) const {
	const vk::PresentInfoKHR presentInfo(
	    1,
	    &*syncObjects.renderFinishedSemaphore,
	    1,
	    &swapchain,
	    &imageIndex);

	return presentQueue.presentKHR(&presentInfo);
}
