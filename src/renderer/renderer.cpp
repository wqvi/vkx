#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/renderer.hpp>
#include <vkx/renderer/uniform_buffer.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

vkx::VulkanAllocationDeleter::VulkanAllocationDeleter(VmaAllocator allocator)
    : allocator(allocator) {
}

void vkx::VulkanAllocationDeleter::operator()(VmaAllocation allocation) const noexcept {
	if (allocator) {
		vmaFreeMemory(allocator, allocation);
	}
}

vkx::Buffer::Buffer(vk::UniqueBuffer&& buffer, vkx::UniqueVulkanAllocation&& allocation, VmaAllocationInfo&& allocationInfo)
    : buffer(std::move(buffer)), allocation(std::move(allocation)), allocationInfo(std::move(allocationInfo)) {}

vkx::Buffer::operator VkBuffer() const {
	return *buffer;
}

vkx::Image::Image(vk::UniqueImage&& image, vkx::UniqueVulkanAllocation&& allocation)
    : resourceImage(std::move(image)),
      resourceAllocation(std::move(allocation)) {
}

void vkx::VulkanAllocatorDeleter::operator()(VmaAllocator allocator) const noexcept {
	vmaDestroyAllocator(allocator);
}

vkx::VulkanAllocator::VulkanAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice)
    : logicalDevice(logicalDevice) {
	constexpr VmaVulkanFunctions vulkanFunctions{
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

	auto cAllocator = vkx::create<VmaAllocator>(
	    vmaCreateAllocator,
	    [](auto result) {
		    if (result != VK_SUCCESS) {
			    throw std::runtime_error("Failed to create vulkan memory allocator.");
		    }
	    },
	    &allocatorCreateInfo);

	allocator.reset(cAllocator);
}

vkx::VulkanAllocator::operator VmaAllocator() const {
	return allocator.get();
}

vkx::Image vkx::VulkanAllocator::allocateImage(const vkx::CommandSubmitter& commandSubmitter,
					       const std::string& file,
					       vk::Format format,
					       vk::ImageTiling tiling,
					       vk::ImageUsageFlags imageUsage,
					       VmaAllocationCreateFlags flags,
					       VmaMemoryUsage memoryUsage) const {
	int textureWidth = 0;
	int textureHeight = 0;
	int textureChannels = 0;
	auto* pixels = stbi_load(file.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image.");
	}

	const auto imageSize = static_cast<vk::DeviceSize>(textureWidth) * textureHeight * STBI_rgb_alpha;

	const auto stagingBuffer = allocateBuffer(pixels, imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	const vk::Extent3D imageExtent{static_cast<std::uint32_t>(textureWidth), static_cast<std::uint32_t>(textureHeight), 1};

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
	if (vmaCreateImage(allocator.get(), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &resourceImage, &resourceAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	commandSubmitter.transitionImageLayout(resourceImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	commandSubmitter.copyBufferToImage(static_cast<VkBuffer>(stagingBuffer), resourceImage, textureWidth, textureHeight);

	commandSubmitter.transitionImageLayout(resourceImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stbi_image_free(pixels);
	return vkx::Image{vk::UniqueImage(resourceImage, logicalDevice), UniqueVulkanAllocation(resourceAllocation, VulkanAllocationDeleter{allocator.get()})};
}

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
	    Access::eNone,
	    Access::eColorAttachmentWrite | Access::eDepthStencilAttachmentWrite};

	const std::array renderPassAttachments = {colorAttachment, depthAttachment};

	const vk::RenderPassCreateInfo renderPassCreateInfo{
	    {},
	    renderPassAttachments,
	    subpass,
	    dependency};

	renderPass = logicalDevice.createRenderPassUnique(renderPassCreateInfo);
}

vkx::VulkanRenderPass::operator VkRenderPass() const {
	return *renderPass;
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

vkx::VulkanDevice::operator VkDevice() const {
	return *logicalDevice;
}

vkx::QueueConfig vkx::VulkanDevice::getQueueConfig() const {
	return vkx::QueueConfig{physicalDevice, surface};
}

vkx::SwapchainInfo vkx::VulkanDevice::getSwapchainInfo() const {
	return vkx::SwapchainInfo{physicalDevice, surface};
}

vkx::VulkanRenderPass vkx::VulkanDevice::createRenderPass(vk::Format colorFormat, vk::AttachmentLoadOp loadOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) const {
	return vkx::VulkanRenderPass{*logicalDevice, static_cast<vk::Format>(findDepthFormat()), colorFormat, loadOp, initialLayout, finalLayout};
}

vkx::VulkanAllocator vkx::VulkanDevice::createAllocator() const {
	return vkx::VulkanAllocator{instance, physicalDevice, *logicalDevice};
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
	    static_cast<VkSharingMode>(imageSharingMode),
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
	    *logicalDevice, &swapchainCreateInfo, nullptr);

	return vkx::Swapchain{*logicalDevice, static_cast<VkRenderPass>(renderPass), static_cast<VmaAllocator>(allocator), swapchain, actualExtent, info.surfaceFormat.format, findDepthFormat()};
}

vkx::CommandSubmitter vkx::VulkanDevice::createCommandSubmitter() const {
	return vkx::CommandSubmitter{physicalDevice, *logicalDevice, surface};
}

vkx::GraphicsPipeline vkx::VulkanDevice::createGraphicsPipeline(const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, const vkx::GraphicsPipelineInformation& information) const {
	return vkx::GraphicsPipeline{*logicalDevice, static_cast<VkRenderPass>(renderPass), static_cast<VmaAllocator>(allocator), information};
}

void vkx::VulkanDevice::waitIdle() const {
	logicalDevice->waitIdle();
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
	    [](auto a) { return a != SDL_TRUE; },
	    this->window);
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

	constexpr auto debugMessageSeverity = Severity::eInfo | Severity::eVerbose | Severity::eWarning | Severity::eError;
	constexpr auto debugMessageType = Type::eGeneral | Type::eValidation | Type::ePerformance;

	constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
	    {},
	    debugMessageSeverity,
	    debugMessageType,
	    [](auto, auto, const auto* pCallbackData, auto*) { SDL_Log("%s", pCallbackData->pMessage); return VK_FALSE; }};

#else
	const auto instanceExtensions = vkx::getArray<const char*>(
	    "Failed to enumerate vulkan extensions",
	    SDL_Vulkan_GetInstanceExtensions,
	    [](auto a) { return a != SDL_TRUE; },
	    this->window);
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

vkx::Mesh::Mesh(std::vector<vkx::Vertex>&& vertices, std::vector<std::uint32_t>&& indices, std::size_t activeIndexCount, const vkx::VulkanAllocator& allocator)
    : vertexBuffer(allocator.allocateBuffer(vertices.data(), vertices.size() * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(allocator.allocateBuffer(indices.data(), indices.size() * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(std::move(vertices)),
      indices(std::move(indices)),
      activeIndexCount(activeIndexCount) {
}

vkx::Mesh::Mesh(std::size_t vertexCount, std::size_t indexCount, const vkx::VulkanAllocator& allocator)
    : vertexBuffer(allocator.allocateBuffer(vertexCount * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      indexBuffer(allocator.allocateBuffer(indexCount * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      vertices(vertexCount),
      indices(indexCount) {
}
