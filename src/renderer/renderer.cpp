#include <vkx/renderer/renderer.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

VkInstance vkx::createInstance(SDL_Window* const window) {
	constexpr VkApplicationInfo applicationInfo{
	    VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    nullptr,
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 1),
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 1),
	    VK_API_VERSION_1_0};

	auto instanceExtensions = getArray<const char*>(
	    "Failed to enumerate vulkan extensions", SDL_Vulkan_GetInstanceExtensions, [](auto a) { return a != SDL_TRUE; }, window);

#ifdef DEBUG
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

#ifdef DEBUG
	constexpr std::array instanceLayers{"VK_LAYER_KHRONOS_validation"};
#else
	constexpr std::array<const char*, 0> instanceLayers{};
#endif

	VkInstanceCreateInfo instanceCreateInfo{
	    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    nullptr,
	    0,
	    &applicationInfo,
	    static_cast<std::uint32_t>(instanceLayers.size()),
	    instanceLayers.data(),
	    static_cast<std::uint32_t>(instanceExtensions.size()),
	    instanceExtensions.data()};

#ifdef DEBUG
	constexpr auto severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	constexpr auto type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	constexpr VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
	    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	    nullptr,
	    0,
	    severity,
	    type,
	    [](auto, auto, const auto* pCallbackData, auto*) { SDL_Log("%s", pCallbackData->pMessage); return VK_FALSE; },
	    nullptr};

	instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
#endif

	return create<VkInstance>(
	    vkCreateInstance, [](auto result) { if (result == VK_ERROR_LAYER_NOT_PRESENT) {
		throw std::runtime_error("Instance layer not present.");
	}

	if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		throw std::runtime_error("Instance extension not present.");
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failure to create instance.");
	} }, &instanceCreateInfo, nullptr);
}

VkSurfaceKHR vkx::createSurface(SDL_Window* const window, VkInstance instance) {
	VkSurfaceKHR surface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE) {
		throw std::runtime_error("Failed to create vulkan surface.");
	}
	return surface;
}

VkPhysicalDevice vkx::getBestPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
	std::uint32_t count = 0;
	if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failure to get count of physical devices.");
	}

	std::vector<VkPhysicalDevice> physicalDevices{count};
	if (vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failure to enumerate physical devices.");
	}

	std::optional<VkPhysicalDevice> physicalDevice;
	std::uint32_t bestRating = 0;
	for (VkPhysicalDevice pDevice : physicalDevices) {
		std::uint32_t currentRating = 0;

		const vkx::QueueConfig indices{pDevice, surface};
		if (indices.isComplete()) {
			currentRating++;
		}

		const vkx::SwapchainInfo info{pDevice, surface};
		if (info.isComplete()) {
			currentRating++;
		}

		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceFeatures(pDevice, &features);

		if (features.samplerAnisotropy) {
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

VkDevice vkx::createDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice) {
	const QueueConfig queueConfig{physicalDevice, surface};

	constexpr float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (const std::uint32_t index : queueConfig.indices) {
		const VkDeviceQueueCreateInfo queueCreateInfo{
		    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		    nullptr,
		    0,
		    index,
		    1,
		    &queuePriority};

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

#ifdef DEBUG
	constexpr std::array layers = {"VK_LAYER_KHRONOS_validation"};
#elif defined(RELEASE)
	constexpr std::array<const char*, 0> layers = {};
#endif

	constexpr std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	const VkDeviceCreateInfo deviceCreateInfo{
	    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    nullptr,
	    0,
	    static_cast<std::uint32_t>(queueCreateInfos.size()),
	    queueCreateInfos.data(),
	    static_cast<std::uint32_t>(layers.size()),
	    layers.data(),
	    static_cast<std::uint32_t>(extensions.size()),
	    extensions.data(),
	    &features};

	VkDevice device = nullptr;
	const auto result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
	if (result == VK_ERROR_LAYER_NOT_PRESENT) {
		throw std::runtime_error("Device layer not present.");
	}

	if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		throw std::runtime_error("Device extension not present.");
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failure to create logical device.");
	}
	return device;
}

VkFormat vkx::findSupportedFormat(VkPhysicalDevice physicalDevice, VkImageTiling tiling, VkFormatFeatureFlags features, const std::vector<VkFormat>& candidates) {
	for (const auto format : candidates) {
		VkFormatProperties formatProps{};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

		const bool isLinear = tiling == VK_IMAGE_TILING_LINEAR && (formatProps.linearTilingFeatures & features) == features;
		const bool isOptimal = tiling == VK_IMAGE_TILING_OPTIMAL && (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

VkImageView vkx::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	const VkImageSubresourceRange subresourceRange{
	    aspectFlags,
	    0,
	    1,
	    0,
	    1};

	const VkImageViewCreateInfo imageViewCreateInfo{
	    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	    nullptr,
	    0,
	    image,
	    VK_IMAGE_VIEW_TYPE_2D,
	    format,
	    {},
	    subresourceRange};

	VkImageView imageView = nullptr;
	if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView)) {
		throw std::runtime_error("Failed to create image view.");
	}

	return imageView;
}

VkSampler vkx::createTextureSampler(VkDevice device, float samplerAnisotropy) {
	const VkSamplerCreateInfo samplerCreateInfo{
	    VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	    nullptr,
	    0,
	    VK_FILTER_LINEAR,
	    VK_FILTER_LINEAR,
	    VK_SAMPLER_MIPMAP_MODE_LINEAR,
	    VK_SAMPLER_ADDRESS_MODE_REPEAT,
	    VK_SAMPLER_ADDRESS_MODE_REPEAT,
	    VK_SAMPLER_ADDRESS_MODE_REPEAT,
	    {},
	    true,
	    samplerAnisotropy,
	    false,
	    VK_COMPARE_OP_ALWAYS,
	    {},
	    {},
	    VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	    false};

	VkSampler sampler = nullptr;
	if (vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler");
	}

	return sampler;
}

VkRenderPass vkx::createRenderPass(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp) {
	const VkAttachmentDescription colorAttachment{
	    0,
	    format,
	    VK_SAMPLE_COUNT_1_BIT,
	    loadOp,
	    VK_ATTACHMENT_STORE_OP_STORE,
	    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    initialLayout,
	    finalLayout};

	const VkAttachmentReference colorAttachmentRef{
	    0,
	    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

	const VkAttachmentDescription depthAttachment{
	    0,
	    findDepthFormat(physicalDevice),
	    VK_SAMPLE_COUNT_1_BIT,
	    VK_ATTACHMENT_LOAD_OP_CLEAR,
	    VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    VK_IMAGE_LAYOUT_UNDEFINED,
	    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

	const VkAttachmentReference depthAttachmentRef{
	    1,
	    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

	const VkSubpassDescription subpass{
	    0,
	    VK_PIPELINE_BIND_POINT_GRAPHICS,
	    0,
	    nullptr,
	    1,
	    &colorAttachmentRef,
	    nullptr,
	    &depthAttachmentRef,
	    0,
	    nullptr};

	const VkSubpassDependency dependency{
	    VK_SUBPASS_EXTERNAL,
	    0,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	    0,
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};

	const std::array renderPassAttachments = {colorAttachment, depthAttachment};

	const VkRenderPassCreateInfo renderPassCreateInfo{
	    VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	    nullptr,
	    0,
	    static_cast<std::uint32_t>(renderPassAttachments.size()),
	    renderPassAttachments.data(),
	    1,
	    &subpass,
	    1,
	    &dependency};

	VkRenderPass renderPass = nullptr;
	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass.");
	}

	return renderPass;
}

[[nodiscard]] std::vector<vkx::SyncObjects> vkx::createSyncObjects(VkDevice device) {
	std::vector<vkx::SyncObjects> objs{vkx::MAX_FRAMES_IN_FLIGHT};

	std::generate(objs.begin(), objs.end(), [&device]() { return vkx::SyncObjects{device}; });

	return objs;
}

VkSwapchainKHR vkx::createSwapchain(VkDevice device, VkSurfaceKHR surface, SDL_Window* window, const vkx::SwapchainInfo& info, const vkx::QueueConfig& config) {
	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);

	const auto surfaceFormat = info.chooseSurfaceFormat();
	const auto presentMode = info.choosePresentMode();
	const auto actualExtent = info.chooseExtent(width, height);
	const auto imageCount = info.getImageCount();
	const auto imageSharingMode = config.getImageSharingMode();

	const VkSwapchainCreateInfoKHR swapchainCreateInfo{
	    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    nullptr,
	    0,
	    surface,
	    imageCount,
	    static_cast<VkFormat>(surfaceFormat.format),
	    static_cast<VkColorSpaceKHR>(surfaceFormat.colorSpace),
	    static_cast<VkExtent2D>(actualExtent),
	    1,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    static_cast<VkSharingMode>(imageSharingMode),
	    static_cast<std::uint32_t>(config.indices.size()),
	    config.indices.data(),
	    static_cast<VkSurfaceTransformFlagBitsKHR>(info.capabilities.currentTransform),
	    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    static_cast<VkPresentModeKHR>(presentMode),
	    true};

	VkSwapchainKHR swapchain = nullptr;
	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain)) {
		throw std::runtime_error("Failed to create swapchain");
	}

	return swapchain;
}

VmaAllocator vkx::createAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance) {
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

	VmaAllocator allocator = nullptr;
	if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan memory allocator.");
	}

	return allocator;
}

VmaAllocation vkx::allocateImage(VmaAllocationInfo* allocationInfo, VkImage* image, VmaAllocator allocator, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) {
	const VkExtent3D imageExtent{width, height, 1};

	const VkImageCreateInfo imageCreateInfo{
	    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	    nullptr,
	    0,
	    VK_IMAGE_TYPE_2D,
	    format,
	    imageExtent,
	    1,
	    1,
	    VK_SAMPLE_COUNT_1_BIT,
	    tiling,
	    imageUsage,
	    VK_SHARING_MODE_EXCLUSIVE,
	    0,
	    nullptr,
	    VK_IMAGE_LAYOUT_UNDEFINED};

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = flags;
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

	VmaAllocation allocation = nullptr;
	if (vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, image, &allocation, allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	return allocation;
}

[[nodiscard]] VmaAllocation vkx::allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) {
	const VkBufferCreateInfo bufferCreateInfo{
	    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    nullptr,
	    0,
	    size,
	    static_cast<VkBufferUsageFlags>(bufferUsage),
	    VK_SHARING_MODE_EXCLUSIVE,
	    0,
	    nullptr};

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = flags;
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

	VmaAllocation allocation = nullptr;
	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, buffer, &allocation, allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	return allocation;
}

VmaAllocation vkx::allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, const void* ptr, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) {
	const VkBufferCreateInfo bufferCreateInfo{
	    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    nullptr,
	    0,
	    size,
	    static_cast<VkBufferUsageFlags>(bufferUsage),
	    VK_SHARING_MODE_EXCLUSIVE,
	    0,
	    nullptr};

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = flags;
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = {};

	VmaAllocation allocation = nullptr;
	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, buffer, &allocation, allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	std::memcpy(allocationInfo->pMappedData, ptr, allocationInfo->size);

	return allocation;
}

std::vector<vkx::UniformBuffer> vkx::allocateUniformBuffers(VmaAllocator allocator, std::size_t size) {
	std::vector<vkx::UniformBuffer> buffers;
	buffers.reserve(MAX_FRAMES_IN_FLIGHT);
	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkBuffer buffer = nullptr;
		VmaAllocationInfo allocationInfo{};
		auto allocation = vkx::allocateBuffer(&allocationInfo, &buffer, allocator, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		buffers.emplace_back(allocator, buffer, allocation, allocationInfo);
	}
	return buffers;
}