#include "vkx/pch.hpp"
#include <stdexcept>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

VkInstance vkx::createInstance(SDL_Window* const window) {
	constexpr VkApplicationInfo ai{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"VKX",
		VK_MAKE_VERSION(0, 0, 1),
		"VKX",
		VK_MAKE_VERSION(0, 0, 1), 
		VK_API_VERSION_1_0};

	std::uint32_t count = 0;
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr) != SDL_TRUE) {
		throw std::runtime_error("Failed to enumerate vulkan extensions");
	}

	std::vector<const char*> instanceExtensions{count};
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, instanceExtensions.data()) != SDL_TRUE) {
		throw std::runtime_error("Failed to enumerate vulkan extensions");
	}

#ifdef DEBUG
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

#ifdef DEBUG
	constexpr std::array instanceLayers{"VK_LAYER_KHRONOS_validation"};
#else
	constexpr std::array<const char*, 0> instanceLayers{};
#endif

	VkInstanceCreateInfo icf{
	    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&ai,
		static_cast<std::uint32_t>(instanceLayers.size()),
		instanceLayers.data(),
		static_cast<std::uint32_t>(instanceExtensions.size()),
		instanceExtensions.data()};

#ifdef DEBUG
	constexpr auto severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	constexpr auto type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	constexpr VkDebugUtilsMessengerCreateInfoEXT dumci{
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, 
		nullptr, 
		0, 
		severity, 
		type, 
		[](auto, auto, const auto* pCallbackData, auto*) { SDL_Log("%s", pCallbackData->pMessage); return VK_FALSE; }, 
		nullptr};

	icf.pNext = &dumci;
#endif
	VkInstance instance = nullptr;
	const auto result = vkCreateInstance(&icf, nullptr, &instance);
	if (result == VK_ERROR_LAYER_NOT_PRESENT) {
		throw std::runtime_error("Layer not present");	
	}

	if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		throw std::runtime_error("Extension not present");
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failure to create instance");
	}

	return instance;
}

vk::UniqueSurfaceKHR vkx::createSurface(SDL_Window* const window, vk::Instance instance) {
	VkSurfaceKHR cSurface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, instance, &cSurface) != SDL_TRUE) {
		throw std::runtime_error("Failed to create vulkan surface.");
	}
	return vk::UniqueSurfaceKHR{cSurface, vk::Instance{instance}};
}

vk::PhysicalDevice vkx::getBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface) {
	const auto physicalDevices = instance.enumeratePhysicalDevices();

	std::optional<vk::PhysicalDevice> physicalDevice;
	std::uint32_t bestRating = 0;
	for (vk::PhysicalDevice pDevice : physicalDevices) {
		std::uint32_t currentRating = 0;

		const vkx::QueueConfig indices{pDevice, surface};
		if (indices.isComplete()) {
			currentRating++;
		}

		const vkx::SwapchainInfo info{pDevice, surface};
		if (info.isComplete()) {
			currentRating++;
		}

		if (pDevice.getFeatures().samplerAnisotropy) {
			currentRating++;
		}

		if (currentRating > bestRating) {
			bestRating = currentRating;
			physicalDevice = pDevice;
		}
	}

	if (!physicalDevice.has_value()) {
		throw std::runtime_error("Failure to find suitable physical device!");
	}

	return *physicalDevice;
}

vk::UniqueDevice vkx::createDevice(vk::Instance instance, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice) {
	const QueueConfig queueConfig{physicalDevice, surface};
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

	return physicalDevice.createDeviceUnique(deviceCreateInfo);
}

vk::Format vkx::findSupportedFormat(vk::PhysicalDevice physicalDevice, vk::ImageTiling tiling, vk::FormatFeatureFlags features, const std::vector<vk::Format>& candidates) {
	for (const vk::Format format : candidates) {
		const auto formatProps = physicalDevice.getFormatProperties(format);

		const bool isLinear = tiling == vk::ImageTiling::eLinear && (formatProps.linearTilingFeatures & features) == features;
		const bool isOptimal = tiling == vk::ImageTiling::eOptimal && (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return vk::Format::eUndefined;
}

vk::UniqueImageView vkx::createImageViewUnique(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) {
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

	return device.createImageViewUnique(imageViewInfo);
}

vk::UniqueSampler vkx::createTextureSamplerUnique(vk::Device device, float samplerAnisotropy) {
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
	    samplerAnisotropy,
	    false,
	    vk::CompareOp::eAlways,
	    {},
	    {},
	    vk::BorderColor::eIntOpaqueBlack,
	    false};

	return device.createSamplerUnique(samplerInfo);
}

vk::UniqueRenderPass vkx::createRenderPassUnique(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Format format, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout, vk::AttachmentLoadOp loadOp) {
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
	    findDepthFormat(physicalDevice),
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

	return device.createRenderPassUnique(renderPassInfo);
}

[[nodiscard]] std::vector<vkx::SyncObjects> vkx::createSyncObjects(vk::Device device) {
	std::vector<vkx::SyncObjects> objs{vkx::MAX_FRAMES_IN_FLIGHT};

	std::generate(objs.begin(), objs.end(), [&device]() { return vkx::SyncObjects{device}; });

	return objs;
}

vk::UniqueSwapchainKHR vkx::createSwapchainUnique(vk::Device device, vk::SurfaceKHR surface, SDL_Window* window, const vkx::SwapchainInfo& info, const vkx::QueueConfig& config) {
	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);

	const auto surfaceFormat = info.chooseSurfaceFormat();
	const auto presentMode = info.choosePresentMode();
	const auto actualExtent = info.chooseExtent(width, height);
	const auto imageCount = info.getImageCount();
	const auto imageSharingMode = config.getImageSharingMode();

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo{
	    {},
	    surface,
	    imageCount,
	    surfaceFormat.format,
	    surfaceFormat.colorSpace,
	    actualExtent,
	    1,
	    vk::ImageUsageFlagBits::eColorAttachment,
	    imageSharingMode,
	    config.indices,
	    info.capabilities.currentTransform,
	    vk::CompositeAlphaFlagBitsKHR::eOpaque,
	    presentMode,
	    true};

	return device.createSwapchainKHRUnique(swapchainCreateInfo);
}