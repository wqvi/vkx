#include <vkx/renderer/buffers.hpp>
#include <vkx/renderer/commands.hpp>
#include <vkx/renderer/image.hpp>
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/renderer.hpp>
#include <vkx/renderer/swapchain.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef DEBUG
static constexpr std::array<const char*, 1> layers{"VK_LAYER_KHRONOS_validation"};
#else
static constexpr std::array<const char*, 0> layers{};
#endif

vkx::VulkanRenderPass::VulkanRenderPass(vk::Device logicalDevice, vk::Format depthFormat, vk::Format colorFormat, vk::AttachmentLoadOp loadOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) {
	using Sample = vk::SampleCountFlagBits;
	using Load = vk::AttachmentLoadOp;
	using Store = vk::AttachmentStoreOp;
	using Layout = vk::ImageLayout;
	using Stage = vk::PipelineStageFlagBits;
	using Access = vk::AccessFlagBits;

	const vk::AttachmentDescription colorAttachment{
	    {},
	    colorFormat,
	    Sample::e1,
	    loadOp,
	    Store::eStore,
	    Load::eDontCare,
	    Store::eDontCare,
	    initialLayout,
	    finalLayout};

	const vk::AttachmentReference colorAttachmentRef{
	    0,
	    Layout::eColorAttachmentOptimal};

	const vk::AttachmentDescription depthAttachment{
	    {},
	    depthFormat,
	    Sample::e1,
	    Load::eClear,
	    Store::eDontCare,
	    Load::eDontCare,
	    Store::eDontCare,
	    Layout::eUndefined,
	    Layout::eDepthStencilAttachmentOptimal};

	const vk::AttachmentReference depthAttachmentRef{
	    1,
	    Layout::eDepthStencilAttachmentOptimal};

	const vk::SubpassDescription subpass{
	    {},
	    vk::PipelineBindPoint::eGraphics,
	    {},
	    colorAttachmentRef,
	    {},
	    &depthAttachmentRef};

	const vk::SubpassDependency dependency{
	    VK_SUBPASS_EXTERNAL,
	    UINT32_C(0),
	    Stage::eColorAttachmentOutput | Stage::eEarlyFragmentTests,
	    Stage::eColorAttachmentOutput | Stage::eEarlyFragmentTests,
	    {},
	    Access::eColorAttachmentWrite | Access::eDepthStencilAttachmentWrite};

	const std::array renderPassAttachments = {colorAttachment, depthAttachment};

	const vk::RenderPassCreateInfo renderPassCreateInfo{
	    {},
	    renderPassAttachments,
	    subpass,
	    dependency};

	renderPass = logicalDevice.createRenderPassUnique(renderPassCreateInfo);
}

vkx::VulkanDevice::VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice)
    : instance(instance),
      surface(surface),
      physicalDevice(physicalDevice) {
	const vkx::QueueConfig queueConfig{physicalDevice, surface};

	constexpr float queuePriority = 1.0f;
	const auto queueCreateInfos = queueConfig.createQueueInfos(&queuePriority);

	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	constexpr std::array deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	const vk::DeviceCreateInfo deviceCreateInfo{
	    {},
	    queueCreateInfos,
	    layers,
	    deviceExtensions,
	    &features};

	logicalDevice = physicalDevice.createDeviceUnique(deviceCreateInfo);

	maxSamplerAnisotropy = physicalDevice.getProperties().limits.maxSamplerAnisotropy;
}

vkx::QueueConfig vkx::VulkanDevice::getQueueConfig() const {
	return vkx::QueueConfig{physicalDevice, surface};
}

vkx::SwapchainInfo vkx::VulkanDevice::getSwapchainInfo(const vkx::Window& window) const {
	return vkx::SwapchainInfo{physicalDevice, surface, window};
}

vkx::VulkanRenderPass vkx::VulkanDevice::createRenderPass(vk::Format colorFormat, vk::AttachmentLoadOp loadOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) const {
	return vkx::VulkanRenderPass{*logicalDevice, static_cast<vk::Format>(findDepthFormat()), colorFormat, loadOp, initialLayout, finalLayout};
}

vkx::VulkanAllocator vkx::VulkanDevice::createAllocator() const {
	return vkx::VulkanAllocator{instance, physicalDevice, *logicalDevice};
}

vk::Format vkx::VulkanDevice::findSupportedFormat(vk::ImageTiling tiling, vk::FormatFeatureFlags features, const std::vector<vk::Format>& candidates) const {
	for (const auto format : candidates) {
		const auto formatProps = physicalDevice.getFormatProperties(format);

		const bool isLinear = tiling == vk::ImageTiling::eLinear && (formatProps.linearTilingFeatures & features) == features;
		const bool isOptimal = tiling == vk::ImageTiling::eOptimal && (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return vk::Format::eUndefined;
}

vkx::Swapchain vkx::VulkanDevice::createSwapchain(const vkx::VulkanAllocator& allocator, const vkx::VulkanRenderPass& renderPass, const vkx::Window& window) const {
	const auto info = getSwapchainInfo(window);
	const auto config = getQueueConfig();

	const auto [width, height] = window.getDimensions();

	const auto imageSharingMode = config.getImageSharingMode();

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo{
	    {},
	    surface,
	    info.imageCount,
	    info.surfaceFormat,
	    info.surfaceColorSpace,
	    info.actualExtent,
	    1,
	    vk::ImageUsageFlagBits::eColorAttachment,
	    imageSharingMode,
	    config.indices,
	    info.currentTransform,
	    vk::CompositeAlphaFlagBitsKHR::eOpaque,
	    info.presentMode,
	    true};

	return vkx::Swapchain{*this, renderPass, allocator, info, logicalDevice->createSwapchainKHRUnique(swapchainCreateInfo)};
}

vkx::CommandSubmitter vkx::VulkanDevice::createCommandSubmitter() const {
	return vkx::CommandSubmitter{physicalDevice, *logicalDevice, surface};
}

vkx::pipeline::GraphicsPipeline vkx::VulkanDevice::createGraphicsPipeline(const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, const vkx::pipeline::GraphicsPipelineInformation& information) const {
	return vkx::pipeline::GraphicsPipeline{*logicalDevice, *renderPass.renderPass, allocator, information};
}

vkx::pipeline::ComputePipeline vkx::VulkanDevice::createComputePipeline(const vkx::pipeline::ComputePipelineInformation& information) const {
	return vkx::pipeline::ComputePipeline{*logicalDevice, information};
}

std::vector<vkx::SyncObjects> vkx::VulkanDevice::createSyncObjects() const {
	std::vector<vkx::SyncObjects> objs{vkx::MAX_FRAMES_IN_FLIGHT};

	std::generate(objs.begin(), objs.end(), [&logicalDevice = *this->logicalDevice]() { return vkx::SyncObjects{logicalDevice}; });

	return objs;
}

vk::UniqueSampler vkx::VulkanDevice::createTextureSampler() const {
	using Filter = vk::Filter;
	using Address = vk::SamplerAddressMode;

	const vk::SamplerCreateInfo samplerCreateInfo{
	    {},
	    Filter::eLinear,
	    Filter::eLinear,
	    vk::SamplerMipmapMode::eLinear,
	    Address::eRepeat,
	    Address::eRepeat,
	    Address::eRepeat,
	    {},
	    true,
	    maxSamplerAnisotropy,
	    false,
	    vk::CompareOp::eAlways,
	    {},
	    {},
	    vk::BorderColor::eIntOpaqueBlack,
	    false};

	return logicalDevice->createSamplerUnique(samplerCreateInfo);
}

void vkx::VulkanDevice::waitIdle() const {
	logicalDevice->waitIdle();
}

vk::UniqueImageView vkx::VulkanDevice::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const {
	const vk::ImageSubresourceRange subresourceRange{
	    aspectFlags,
	    0,
	    1,
	    0,
	    1};

	const vk::ImageViewCreateInfo imageViewCreateInfo{
	    {},
	    image,
	    vk::ImageViewType::e2D,
	    format,
	    {},
	    subresourceRange};

	return logicalDevice->createImageViewUnique(imageViewCreateInfo);
}

vkx::VulkanInstance::VulkanInstance(const vkx::Window& window)
    : window(static_cast<SDL_Window*>(window)) {
	constexpr vk::ApplicationInfo applicationInfo{
	    "VKX",
	    vkx::VERSION,
	    "VKX",
	    vkx::VERSION,
	    VK_API_VERSION_1_0};

#ifdef DEBUG
	auto instanceExtensions = vkx::getArray<const char*>(
	    "Failed to enumerate vulkan extensions",
	    SDL_Vulkan_GetInstanceExtensions,
	    [](auto result) { 
			return result != SDL_TRUE; 
		});
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

	constexpr auto debugMessageSeverity = Severity::eInfo | Severity::eVerbose | Severity::eWarning | Severity::eError;
	constexpr auto debugMessageType = Type::eGeneral | Type::eValidation | Type::ePerformance;

	const auto debugCallback = [](auto, auto, const auto* pCallbackData, auto*) { 
		SDL_Log("%s", pCallbackData->pMessage); return VK_FALSE; 
	};

	constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
	    {},
	    debugMessageSeverity,
	    debugMessageType,
	    debugCallback};

#else
	const auto instanceExtensions = vkx::getArray<const char*>(
	    "Failed to enumerate vulkan extensions",
	    SDL_Vulkan_GetInstanceExtensions,
	    [](auto a) { return a != SDL_TRUE; });
#endif

	const vk::InstanceCreateInfo instanceCreateInfo{{}, &applicationInfo, layers, instanceExtensions};

#ifdef DEBUG
	const vk::StructureChain structureChain{instanceCreateInfo, debugUtilsMessengerCreateInfo};
	instance = vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
	instance = vk::createInstanceUnique(instanceCreateInfo);
#endif

	const auto cSurface = vkx::create<VkSurfaceKHR>(
	    SDL_Vulkan_CreateSurface, [](auto result) {
		    if (result != SDL_TRUE) {
			    throw std::runtime_error("Failed to create SDL Vulkan surface.");
		    }
	    },
	    this->window, *instance);

	surface = vk::UniqueSurfaceKHR(cSurface, *instance);
}

vkx::VulkanDevice vkx::VulkanInstance::createDevice() const {
	const auto physicalDevices = instance->enumeratePhysicalDevices();

	vk::PhysicalDevice bestPhysicalDevice = nullptr;
	std::uint32_t bestRating = 0;
	for (vk::PhysicalDevice physicalDevice : physicalDevices) {
		const auto currentRating = ratePhysicalDevice(physicalDevice);

		if (currentRating > bestRating) {
			bestRating = currentRating;
			bestPhysicalDevice = physicalDevice;
		}
	}

	if (!bestPhysicalDevice) {
		throw std::runtime_error("Failure to find suitable physical device!");
	}

	return vkx::VulkanDevice{*instance, *surface, bestPhysicalDevice};
}

std::uint32_t vkx::VulkanInstance::ratePhysicalDevice(vk::PhysicalDevice physicalDevice) const {
	std::uint32_t rating = 0;

	const vkx::QueueConfig indices{physicalDevice, *surface};
	if (indices.isComplete()) {
		rating++;
	}

	const auto features = physicalDevice.getFeatures();
	if (features.samplerAnisotropy) {
		rating++;
	}

	const auto properties = physicalDevice.getProperties();
	if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
		rating++;
	}

	return rating;
}
