#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/renderer.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef DEBUG
static constexpr std::array<const char*, 1> layers{"VK_LAYER_KHRONOS_validation"};
#else
static constexpr std::array<const char*, 0> layers{};
#endif

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

	return vkx::create<VkImageView>(
	    vkCreateImageView,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create image view.");
		    }
	    },
	    device, &imageViewCreateInfo, nullptr);
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

	return vkx::create<VkSampler>(
	    vkCreateSampler,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create texture sampler.");
		    }
	    },
	    device, &samplerCreateInfo, nullptr);
}

std::vector<vkx::SyncObjects> vkx::createSyncObjects(VkDevice device) {
	std::vector<vkx::SyncObjects> objs{vkx::MAX_FRAMES_IN_FLIGHT};

	std::generate(objs.begin(), objs.end(), [&device]() { return vkx::SyncObjects{device}; });

	return objs;
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

VmaAllocation vkx::allocateBuffer(VmaAllocationInfo* allocationInfo, VkBuffer* buffer, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) {
	const VkBufferCreateInfo bufferCreateInfo{
	    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    nullptr,
	    0,
	    size,
	    bufferUsage,
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
	    bufferUsage,
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

vkx::VulkanAllocator::VulkanAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice) {
	const VmaVulkanFunctions vulkanFunctions{
	    &vkGetInstanceProcAddr,
	    &vkGetDeviceProcAddr};

	const VmaAllocatorCreateInfo allocatorCreateInfo{
	    0,
	    physicalDevice,
	    logicalDevice,
	    0,
	    nullptr,
	    nullptr,
	    nullptr,
	    &vulkanFunctions,
	    instance,
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

vkx::VulkanAllocator::VulkanAllocator(VulkanAllocator&& other) noexcept
    : allocator(other.allocator) {
	other.allocator = nullptr;
}

vkx::VulkanAllocator::~VulkanAllocator() {
	if (allocator) {
		vmaDestroyAllocator(allocator);
	}
}

vkx::VulkanAllocator& vkx::VulkanAllocator::operator=(VulkanAllocator&& other) noexcept {
	allocator = other.allocator;

	other.allocator = nullptr;
	return *this;
}

vkx::VulkanAllocator::operator VmaAllocator() const {
	return allocator;
}

vkx::Buffer vkx::VulkanAllocator::allocateBuffer(const void* data, std::size_t memorySize, VkBufferUsageFlags bufferFlags, VmaAllocationCreateFlags allocationFlags, VmaMemoryUsage memoryUsage) const {
	return vkx::Buffer{allocator, data, memorySize, bufferFlags, allocationFlags, memoryUsage};
}

vkx::VulkanRenderPass::VulkanRenderPass(VkDevice logicalDevice, VkFormat depthFormat, VkFormat colorFormat, VkAttachmentLoadOp loadOp, VkImageLayout initialLayout, VkImageLayout finalLayout)
    : logicalDevice(logicalDevice) {
	const VkAttachmentDescription colorAttachment{
	    0,
	    colorFormat,
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
	    depthFormat,
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

	renderPass = vkx::create<VkRenderPass>(
	    vkCreateRenderPass,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create render pass.");
		    }
	    },
	    logicalDevice, &renderPassCreateInfo, nullptr);
}

vkx::VulkanRenderPass::VulkanRenderPass(VulkanRenderPass&& other) noexcept
    : logicalDevice(other.logicalDevice),
      renderPass(other.renderPass) {
	other.logicalDevice = nullptr;
	other.renderPass = nullptr;
}

vkx::VulkanRenderPass::~VulkanRenderPass() {
	if (renderPass) {
		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
	}
}

vkx::VulkanRenderPass& vkx::VulkanRenderPass::operator=(VulkanRenderPass&& other) noexcept {
	logicalDevice = other.logicalDevice;
	renderPass = other.renderPass;

	other.logicalDevice = nullptr;
	other.renderPass = nullptr;
	return *this;
}

vkx::VulkanRenderPass::operator VkRenderPass() const {
	return renderPass;
}

vkx::VulkanDevice::VulkanDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice)
    : instance(instance),
      surface(surface),
      physicalDevice(physicalDevice) {
	const vkx::QueueConfig queueConfig{physicalDevice, surface};

	constexpr float queuePriority = 1.0f;
	const auto queueCreateInfos = queueConfig.createQueueInfos(&queuePriority);

	VkPhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	constexpr std::array deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	const VkDeviceCreateInfo deviceCreateInfo{
	    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    nullptr,
	    0,
	    static_cast<std::uint32_t>(queueCreateInfos.size()),
	    queueCreateInfos.data(),
	    static_cast<std::uint32_t>(layers.size()),
	    layers.data(),
	    static_cast<std::uint32_t>(deviceExtensions.size()),
	    deviceExtensions.data(),
	    &features};

	logicalDevice = vkx::create<VkDevice>(
	    vkCreateDevice, [](auto result) {
		    if (result == VK_ERROR_LAYER_NOT_PRESENT) {
			    throw std::runtime_error("Device layer not present.");
		    }

		    if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			    throw std::runtime_error("Device extension not present.");
		    }

		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failure to create logical device.");
		    }
	    },
	    physicalDevice, &deviceCreateInfo, nullptr);

	const auto properties = vkx::getObject<VkPhysicalDeviceProperties>(vkGetPhysicalDeviceProperties, physicalDevice);
	maxSamplerAnisotropy = properties.limits.maxSamplerAnisotropy;
}

vkx::VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept
    : instance(other.instance),
      surface(other.surface),
      physicalDevice(other.physicalDevice),
      logicalDevice(other.logicalDevice),
      maxSamplerAnisotropy(other.maxSamplerAnisotropy) {
	other.instance = nullptr;
	other.surface = nullptr;
	other.physicalDevice = nullptr;
	other.logicalDevice = nullptr;
	other.maxSamplerAnisotropy = 0.0f;
}

vkx::VulkanDevice::~VulkanDevice() {
	if (logicalDevice) {
		vkDestroyDevice(logicalDevice, nullptr);
	}
}

vkx::VulkanDevice& vkx::VulkanDevice::operator=(VulkanDevice&& other) noexcept {
	instance = other.instance;
	surface = other.surface;
	physicalDevice = other.physicalDevice;
	logicalDevice = other.logicalDevice;
	maxSamplerAnisotropy = other.maxSamplerAnisotropy;

	other.instance = nullptr;
	other.surface = nullptr;
	other.physicalDevice = nullptr;
	other.logicalDevice = nullptr;
	other.maxSamplerAnisotropy = 0.0f;
	return *this;
}

vkx::VulkanDevice::operator VkDevice() const {
	return logicalDevice;
}

vkx::QueueConfig vkx::VulkanDevice::getQueueConfig() const {
	return vkx::QueueConfig{physicalDevice, surface};
}

vkx::SwapchainInfo vkx::VulkanDevice::getSwapchainInfo() const {
	return vkx::SwapchainInfo{physicalDevice, surface};
}

vkx::VulkanRenderPass vkx::VulkanDevice::createRenderPass(VkFormat colorFormat, VkAttachmentLoadOp loadOp, VkImageLayout initialLayout, VkImageLayout finalLayout) const {
	return vkx::VulkanRenderPass{logicalDevice, findDepthFormat(), colorFormat, loadOp, initialLayout, finalLayout};
}

vkx::VulkanAllocator vkx::VulkanDevice::createAllocator() const {
	return vkx::VulkanAllocator{instance, physicalDevice, logicalDevice};
}

VkFormat vkx::VulkanDevice::findSupportedFormat(VkImageTiling tiling, VkFormatFeatureFlags features, const std::vector<VkFormat>& candidates) const {
	for (const auto format : candidates) {
		const auto formatProps = vkx::getObject<VkFormatProperties>(vkGetPhysicalDeviceFormatProperties, physicalDevice, format);

		const bool isLinear = tiling == VK_IMAGE_TILING_LINEAR && (formatProps.linearTilingFeatures & features) == features;
		const bool isOptimal = tiling == VK_IMAGE_TILING_OPTIMAL && (formatProps.optimalTilingFeatures & features) == features;

		if (isLinear || isOptimal) {
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

float vkx::VulkanDevice::getMaxSamplerAnisotropy() const {
	return maxSamplerAnisotropy;
}

vkx::Swapchain vkx::VulkanDevice::createSwapchain(const vkx::VulkanAllocator& allocator, const vkx::VulkanRenderPass& renderPass, const vkx::Window& window) const {
	const auto info = getSwapchainInfo();
	const auto config = getQueueConfig();

	const auto [width, height] = window.getDimensions();

	const auto surfaceFormat = info.surfaceFormat;
	const auto presentMode = info.presentMode;
	const auto actualExtent = info.chooseExtent(width, height);
	const auto imageCount = info.imageCount;
	const auto imageSharingMode = config.getImageSharingMode();

	const VkSwapchainCreateInfoKHR swapchainCreateInfo{
	    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    nullptr,
	    0,
	    surface,
	    imageCount,
	    surfaceFormat.format,
	    surfaceFormat.colorSpace,
	    actualExtent,
	    1,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    imageSharingMode,
	    static_cast<std::uint32_t>(config.indices.size()),
	    config.indices.data(),
	    info.capabilities.currentTransform,
	    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    presentMode,
	    true};

	const auto swapchain = vkx::create<VkSwapchainKHR>(
	    vkCreateSwapchainKHR,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create swapchain.");
		    }
	    },
	    logicalDevice, &swapchainCreateInfo, nullptr);

	return vkx::Swapchain{logicalDevice, static_cast<VkRenderPass>(renderPass), static_cast<VmaAllocator>(allocator), swapchain, actualExtent, info.surfaceFormat.format, findDepthFormat()};
}

vkx::CommandSubmitter vkx::VulkanDevice::createCommandSubmitter() const {
	return vkx::CommandSubmitter{physicalDevice, logicalDevice, surface};
}

vkx::GraphicsPipeline vkx::VulkanDevice::createGraphicsPipeline(const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, const vkx::GraphicsPipelineInformation& information) const {
	return vkx::GraphicsPipeline{logicalDevice, static_cast<VkRenderPass>(renderPass), static_cast<VmaAllocator>(allocator), information};
}

void vkx::VulkanDevice::waitIdle() const {
	if (vkDeviceWaitIdle(logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("Failed to wait for device.");
	}
}

vkx::VulkanInstance::VulkanInstance(const vkx::Window& window)
    : window(static_cast<SDL_Window*>(window)) {
	constexpr VkApplicationInfo applicationInfo{
	    VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    nullptr,
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 1),
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 2),
	    VK_API_VERSION_1_0};

#ifdef DEBUG
	constexpr auto messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	constexpr auto messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	constexpr VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
	    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	    nullptr,
	    0,
	    messageSeverity,
	    messageType,
	    [](auto, auto, const auto* pCallbackData, auto*) { SDL_Log("%s", pCallbackData->pMessage); return VK_FALSE; },
	    nullptr};

	const void* pNext = static_cast<const void*>(&debugUtilsMessengerCreateInfo);
#else
	constexpr void* pNext = nullptr;
#endif

#ifdef RELEASE
	const auto instanceExtensions = vkx::getArray<const char*>(
	    "Failed to enumerate vulkan extensions", SDL_Vulkan_GetInstanceExtensions, [](auto a) { return a != SDL_TRUE; }, this->window);
#else
	auto instanceExtensions = vkx::getArray<const char*>(
	    "Failed to enumerate vulkan extensions", SDL_Vulkan_GetInstanceExtensions, [](auto a) { return a != SDL_TRUE; }, this->window);
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	const VkInstanceCreateInfo instanceCreateInfo{
	    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    pNext,
	    0,
	    &applicationInfo,
	    static_cast<std::uint32_t>(layers.size()),
	    layers.data(),
	    static_cast<std::uint32_t>(instanceExtensions.size()),
	    instanceExtensions.data()};

	instance = vkx::create<VkInstance>(
	    vkCreateInstance, [](auto result) {
		    if (result == VK_ERROR_LAYER_NOT_PRESENT) {
			    throw std::runtime_error("Instance layer not present.");
		    }

		    if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			    throw std::runtime_error("Instance extension not present.");
		    }

		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failure to create instance.");
		    }
	    },
	    &instanceCreateInfo, nullptr);

	surface = vkx::create<VkSurfaceKHR>(
	    SDL_Vulkan_CreateSurface, [](auto result) {
		    if (result != SDL_TRUE) {
			    throw std::runtime_error("Failed to create SDL Vulkan surface.");
		    }
	    },
	    this->window, instance);
}

vkx::VulkanInstance::VulkanInstance(VulkanInstance&& other) noexcept
    : window(other.window),
      instance(other.instance),
      surface(other.surface) {
	other.window = nullptr;
	other.instance = nullptr;
	other.surface = nullptr;
}

vkx::VulkanInstance::~VulkanInstance() {
	if (instance && surface) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}
}

vkx::VulkanInstance& vkx::VulkanInstance::operator=(VulkanInstance&& other) noexcept {
	window = other.window;
	instance = other.instance;
	surface = other.surface;

	other.window = nullptr;
	other.instance = nullptr;
	other.surface = nullptr;
	return *this;
}

vkx::VulkanDevice vkx::VulkanInstance::createDevice() const {
	const auto physicalDevices = vkx::getArray<VkPhysicalDevice>(
	    "Failed to enumerate physical devices.",
	    vkEnumeratePhysicalDevices,
	    [](auto a) { return a != VK_SUCCESS; },
	    instance);

	VkPhysicalDevice bestPhysicalDevice = nullptr;
	std::uint32_t bestRating = 0;
	for (VkPhysicalDevice physicalDevice : physicalDevices) {
		std::uint32_t currentRating = ratePhysicalDevice(physicalDevice);

		if (currentRating > bestRating) {
			bestRating = currentRating;
			bestPhysicalDevice = physicalDevice;
		}
	}

	if (!bestPhysicalDevice) {
		throw std::runtime_error("Failure to find suitable physical device!");
	}

	return vkx::VulkanDevice{instance, surface, bestPhysicalDevice};
}

std::uint32_t vkx::VulkanInstance::ratePhysicalDevice(VkPhysicalDevice physicalDevice) const {
	std::uint32_t rating = 0;

	const vkx::QueueConfig indices{physicalDevice, surface};
	if (indices.isComplete()) {
		rating++;
	}

	const auto features = vkx::getObject<VkPhysicalDeviceFeatures>(vkGetPhysicalDeviceFeatures, physicalDevice);
	if (features.samplerAnisotropy) {
		rating++;
	}

	const auto properties = vkx::getObject<VkPhysicalDeviceProperties>(vkGetPhysicalDeviceProperties, physicalDevice);
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		rating++;
	}

	return rating;
}

vkx::Buffer::Buffer(VmaAllocator allocator,
		    const void* data,
		    std::size_t memorySize,
		    VkBufferUsageFlags bufferFlags,
		    VmaAllocationCreateFlags allocationFlags,
		    VmaMemoryUsage memoryUsage)
    : allocator(allocator) {
	const VkBufferCreateInfo bufferCreateInfo{
	    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    nullptr,
	    0,
	    memorySize,
	    bufferFlags,
	    VK_SHARING_MODE_EXCLUSIVE,
	    0,
	    nullptr};

	const VmaAllocationCreateInfo allocationCreateInfo{
	    allocationFlags,
	    memoryUsage,
	    0,
	    0,
	    0,
	    nullptr,
	    nullptr,
	    {}};

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate GPU buffer.");
	}

	if (data != nullptr) {
		std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
	}
}

vkx::Buffer::Buffer(Buffer&& other) noexcept
    : allocator(std::move(other.allocator)),
      buffer(std::move(other.buffer)),
      allocation(std::move(other.allocation)),
      allocationInfo(std::move(other.allocationInfo)) {
	other.allocator = nullptr;
	other.buffer = nullptr;
	other.allocation = nullptr;
	std::memset(&other.allocationInfo, 0, sizeof(VmaAllocationInfo));
}

vkx::Buffer::~Buffer() {
	if (buffer) {
		vmaDestroyBuffer(allocator, buffer, allocation);
	}
}

vkx::Buffer& vkx::Buffer::operator=(Buffer&& other) noexcept {
	allocator = std::move(other.allocator);
	buffer = std::move(other.buffer);
	allocation = std::move(other.allocation);
	allocationInfo = std::move(other.allocationInfo);

	other.allocator = nullptr;
	other.buffer = nullptr;
	other.allocation = nullptr;
	std::memset(&other.allocationInfo, 0, sizeof(VmaAllocationInfo));
	return *this;
}

vkx::Buffer::operator VkBuffer() const {
	return buffer;
}

void vkx::Buffer::mapMemory(const void* data) {
	std::memcpy(allocationInfo.pMappedData, data, allocationInfo.size);
}

vkx::TestMesh::TestMesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanAllocator& allocator)
    : vertexBuffer(allocator.allocateBuffer(vertices.data(), vertices.size() * sizeof(vkx::Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)),
      indexBuffer(allocator.allocateBuffer(indices.data(), indices.size() * sizeof(std::uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT)),
      vertices(std::move(vertices)),
      indices(std::move(indices)),
      activeIndexCount(activeIndexCount) {
}

const vkx::Buffer& vkx::TestMesh::getVertexBuffer() const {
	return vertexBuffer;
}

const vkx::Buffer& vkx::TestMesh::getIndexBuffer() const {
	return indexBuffer;
}

std::size_t vkx::TestMesh::getActiveIndexCount() const {
	return activeIndexCount;
}