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
	const vk::ImageSubresourceRange subresourceRange{
	    aspectFlags,
	    0,
	    1,
	    0,
	    1};

	const vk::ImageViewCreateInfo imageViewInfo{
	    {},
	    image,
	    vk::ImageViewType::e2D,
	    format,
	    {},
	    subresourceRange};

	return device->createImageViewUnique(imageViewInfo);
}

vk::UniqueImageView vkx::Device::createTextureImageViewUnique(vk::Image image) const {
	return createImageViewUnique(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

vk::UniqueSampler vkx::Device::createTextureSamplerUnique() const {
	const vk::SamplerCreateInfo samplerInfo{
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
	    false};

	return device->createSamplerUnique(samplerInfo);
}

vkx::Allocator vkx::Device::createAllocator() const {
	return vkx::Allocator{physicalDevice, *device, instance};
}

vkx::Swapchain vkx::Device::createSwapchain(SDL_Window* window, const vkx::Allocator& allocator) const {
	return vkx::Swapchain{*this, surface, window, allocator};
}

vk::UniqueRenderPass vkx::Device::createRenderPass(vk::Format format, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout, vk::AttachmentLoadOp loadOp) const {
	const vk::AttachmentDescription colorAttachment{
	    {},
	    format,
	    vk::SampleCountFlagBits::e1,
	    loadOp,
	    vk::AttachmentStoreOp::eStore,
	    vk::AttachmentLoadOp::eDontCare,
	    vk::AttachmentStoreOp::eDontCare,
	    initialLayout,
	    finalLayout};

	const vk::AttachmentReference colorAttachmentRef{
	    0,
	    vk::ImageLayout::eColorAttachmentOptimal};

	const vk::AttachmentDescription depthAttachment{
	    {},
	    findDepthFormat(),
	    vk::SampleCountFlagBits::e1,
	    vk::AttachmentLoadOp::eClear,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::AttachmentLoadOp::eDontCare,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::ImageLayout::eUndefined,
	    vk::ImageLayout::eDepthStencilAttachmentOptimal};

	const vk::AttachmentReference depthAttachmentRef{
	    1,
	    vk::ImageLayout::eDepthStencilAttachmentOptimal};

	const vk::SubpassDescription subpass{
	    {},
	    vk::PipelineBindPoint::eGraphics,
	    {},
	    colorAttachmentRef,
	    {},
	    &depthAttachmentRef,
	    {}};

	constexpr auto dependencyStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	constexpr auto dependencyAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	const vk::SubpassDependency dependency{
	    VK_SUBPASS_EXTERNAL,
	    0,
	    dependencyStageMask,
	    dependencyStageMask,
	    {},
	    dependencyAccessMask};

	const auto renderPassAttachments = {colorAttachment, depthAttachment};

	const vk::RenderPassCreateInfo renderPassInfo{
	    {},
	    renderPassAttachments,
	    subpass,
	    dependency};

	return device->createRenderPassUnique(renderPassInfo);
}

vkx::GraphicsPipeline vkx::Device::createGraphicsPipeline(const vkx::GraphicsPipelineInformation& info) const {
	return vkx::GraphicsPipeline{*device, info};
}

vkx::CommandSubmitter vkx::Device::createCommandSubmitter() const {
	return vkx::CommandSubmitter{physicalDevice, *device, surface};
}

std::vector<vkx::SyncObjects> vkx::Device::createSyncObjects() const {
	std::vector<vkx::SyncObjects> objs;
	objs.resize(MAX_FRAMES_IN_FLIGHT);

	std::generate(objs.begin(), objs.end(), [&device = *this->device]() { return SyncObjects{device}; });

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Successfully created renderer sync objects.");

	return objs;
}