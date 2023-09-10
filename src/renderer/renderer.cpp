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
	    }, static_cast<SDL_Window*>(window));
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

	constexpr auto debugMessageSeverity = Severity::eInfo | Severity::eVerbose | Severity::eWarning | Severity::eError;
	constexpr auto debugMessageType = Type::eGeneral | Type::eValidation | Type::ePerformance;

	const auto debugCallback = [](auto, auto, const auto* pCallbackData, auto*) {
		SDL_Log("%s", pCallbackData->pMessage);
		return VK_FALSE;
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
	    [](auto a) { return a != SDL_TRUE; }, static_cast<SDL_Window*>(window));
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

	physicalDevice = bestPhysicalDevice;

	const vkx::QueueConfig queueConfig{physicalDevice, *surface};

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

	depthFormat = findSupportedFormat(vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint});

	constexpr VmaVulkanFunctions vulkanFunctions{
	    &vkGetInstanceProcAddr,
	    &vkGetDeviceProcAddr};

	const VmaAllocatorCreateInfo allocatorCreateInfo{
	    0,
	    static_cast<VkPhysicalDevice>(physicalDevice),
	    static_cast<VkDevice>(*logicalDevice),
	    0,
	    nullptr,
	    nullptr,
	    nullptr,
	    &vulkanFunctions,
	    *instance,
	    VK_API_VERSION_1_0,
#ifdef VMA_EXTERNAL_MEMORY
	    nullptr
#endif
	};

	allocator = vkx::create<VmaAllocator>(
	    vmaCreateAllocator,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create vulkan memory allocator.");
		    }
	    },
	    &allocatorCreateInfo);
}

vk::UniqueRenderPass vkx::VulkanInstance::createRenderPass(vk::AttachmentLoadOp loadOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) const {
	using Sample = vk::SampleCountFlagBits;
	using Load = vk::AttachmentLoadOp;
	using Store = vk::AttachmentStoreOp;
	using Layout = vk::ImageLayout;
	using Stage = vk::PipelineStageFlagBits;
	using Access = vk::AccessFlagBits;

	const vkx::SwapchainInfo swapchainInfo{physicalDevice, *surface, window};

	const vk::AttachmentDescription colorAttachment{
	    {},
	    swapchainInfo.surfaceFormat,
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
	    0,
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

	return logicalDevice->createRenderPassUnique(renderPassCreateInfo);
}

vk::Format vkx::VulkanInstance::findSupportedFormat(vk::ImageTiling tiling, vk::FormatFeatureFlags features, const std::vector<vk::Format>& candidates) const {
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

vkx::Swapchain vkx::VulkanInstance::createSwapchain(const vk::UniqueRenderPass& renderPass) const {
	const vkx::SwapchainInfo info{physicalDevice, *surface, window};
	const vkx::QueueConfig config{physicalDevice, *surface};

	int width;
	int height;
	SDL_GetWindowSizeInPixels(window, &width, &height);

	const auto imageSharingMode = config.getImageSharingMode();

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo{
	    {},
	    *surface,
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

	return vkx::Swapchain{*this, renderPass, info, logicalDevice->createSwapchainKHRUnique(swapchainCreateInfo)};
}

vkx::CommandSubmitter vkx::VulkanInstance::createCommandSubmitter() const {
	return vkx::CommandSubmitter{physicalDevice, *logicalDevice, *surface};
}

vkx::pipeline::GraphicsPipeline vkx::VulkanInstance::createGraphicsPipeline(const vk::UniqueRenderPass& renderPass, const vkx::pipeline::GraphicsPipelineInformation& information) const {
	return vkx::pipeline::GraphicsPipeline{*this, *renderPass, information};
}

std::vector<vkx::SyncObjects> vkx::VulkanInstance::createSyncObjects() const {
	std::vector<vkx::SyncObjects> objs{vkx::MAX_FRAMES_IN_FLIGHT};

	std::generate(objs.begin(), objs.end(), [&logicalDevice = *this->logicalDevice]() { return vkx::SyncObjects{logicalDevice}; });

	return objs;
}

vk::UniqueSampler vkx::VulkanInstance::createTextureSampler() const {
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

void vkx::VulkanInstance::waitIdle() const {
	logicalDevice->waitIdle();
}

vk::UniqueImageView vkx::VulkanInstance::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const {
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

void vkx::VulkanInstance::destroy() const {
	vmaDestroyAllocator(allocator);
}

vkx::Buffer vkx::VulkanInstance::allocateBuffer(std::size_t memorySize,
						 vk::BufferUsageFlags bufferFlags,
						 VmaAllocationCreateFlags allocationFlags,
						 VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo{{}, memorySize, bufferFlags, vk::SharingMode::eExclusive};

	const VmaAllocationCreateInfo allocationCreateInfo{
	    allocationFlags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	VkBuffer cBuffer = nullptr;
	VmaAllocation cAllocation = nullptr;
	VmaAllocationInfo cAllocationInfo;
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &cBuffer, &cAllocation, &cAllocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate GPU buffer.");
	}

	return vkx::Buffer{allocator, cBuffer, cAllocation, cAllocationInfo};
}

vkx::Image vkx::VulkanInstance::allocateImage(vk::Extent2D extent,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags,
					       VmaMemoryUsage memoryUsage) const {
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

	VkImage resourceImage = nullptr;
	VmaAllocation resourceAllocation = nullptr;
	if (vmaCreateImage(allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &resourceImage, &resourceAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	return vkx::Image{*logicalDevice, allocator, resourceImage, resourceAllocation};
}

std::vector<vkx::UniformBuffer> vkx::VulkanInstance::allocateUniformBuffers(std::size_t memorySize, std::size_t amount) const {
	std::vector<vkx::UniformBuffer> uniformBuffers;
	uniformBuffers.reserve(amount);
	for (auto i = 0; i < amount; i++) {
		uniformBuffers.emplace_back(allocateBuffer(memorySize, vk::BufferUsageFlagBits::eUniformBuffer));
	}
	return uniformBuffers;
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
