#include "vkx/renderer/core/renderer_types.hpp"
#include "vkx/renderer/renderer.hpp"
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/core/queue_config.hpp>

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
	constexpr std::array layers = {"VK_LAYER_KHRONOS_validation"};
#elif RELEASE
	constexpr std::array<const char*, 0> layers = {};
#endif

	constexpr std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	const vk::DeviceCreateInfo deviceCreateInfo{
	    {},
	    queueCreateInfos,
	    layers,
	    extensions,
	    &deviceFeatures};

	device = physicalDevice.createDeviceUnique(deviceCreateInfo);
}

const vk::Device* vkx::Device::operator->() const {
	return &*device;
}

vkx::Device::operator vk::Device() const {
	return *device;
}

vk::Format vkx::Device::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const {
	/*for (const vk::Format format : candidates) {
		const auto formatProps = physicalDevice.getFormatProperties(format);

		bool isLinear = tiling == vk::ImageTiling::eLinear && (formatProps.linearTilingFeatures & features) == features;
		bool isOptimal = tiling == vk::ImageTiling::eOptimal && (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return vk::Format::eUndefined;*/
	return vkx::findSupportedFormat(physicalDevice, tiling, features, candidates);
}

vk::UniqueImageView vkx::Device::createTextureImageViewUnique(vk::Image image) const {
	return vkx::createTextureImageViewUnique(*device, image);
}

vk::UniqueSampler vkx::Device::createTextureSamplerUnique() const {
	return vkx::createTextureSamplerUnique(*device, maxSamplerAnisotropy);
}

vkx::Allocator vkx::Device::createAllocator() const {
	return vkx::Allocator{physicalDevice, *device, instance};
}

vkx::Swapchain vkx::Device::createSwapchain(SDL_Window* window, vk::RenderPass renderPass, const vkx::Allocator& allocator) const {
	return vkx::Swapchain{*this, renderPass, surface, window, allocator};
}

vk::UniqueRenderPass vkx::Device::createRenderPass(vk::Format format, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout, vk::AttachmentLoadOp loadOp) const {
	return vkx::createRenderPassUnique(*device, physicalDevice, format, initialLayout, finalLayout, loadOp);
}

std::shared_ptr<vkx::GraphicsPipeline> vkx::Device::createGraphicsPipeline(vk::RenderPass renderPass, const Allocator& allocator, const vkx::GraphicsPipelineInformation& info) const {
	return std::shared_ptr<vkx::GraphicsPipeline>(new vkx::GraphicsPipeline{*device, renderPass, allocator, info});
}

vkx::CommandSubmitter vkx::Device::createCommandSubmitter() const {
	return vkx::CommandSubmitter{physicalDevice, *device, surface};
}

std::vector<vkx::SyncObjects> vkx::Device::createSyncObjects() const {
	return vkx::createSyncObjects(*device);
}
