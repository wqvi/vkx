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

vkx::Buffer::operator vk::Buffer() const {
	return *buffer;
}

std::size_t vkx::Buffer::size() const {
	return allocationInfo.size;
}

vkx::Image::Image(vk::Device logicalDevice, vk::UniqueImage&& image, vkx::UniqueVulkanAllocation&& allocation)
    : logicalDevice(logicalDevice),
      resourceImage(std::move(image)),
      resourceAllocation(std::move(allocation)) {
}

vk::UniqueImageView vkx::Image::createView(vk::Format format, vk::ImageAspectFlags aspectFlags) const {
	const vk::ImageSubresourceRange subresourceRange{
	    aspectFlags,
	    0,
	    1,
	    0,
	    1};

	const vk::ImageViewCreateInfo imageViewCreateInfo{
	    {},
	    *resourceImage,
	    vk::ImageViewType::e2D,
	    format,
	    {},
	    subresourceRange};

	return logicalDevice.createImageViewUnique(imageViewCreateInfo);
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

vkx::Buffer vkx::VulkanAllocator::allocateBuffer(std::size_t memorySize,
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
	if (vmaCreateBuffer(allocator.get(), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &cBuffer, &cAllocation, &cAllocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate GPU buffer.");
	}

	return vkx::Buffer{vk::UniqueBuffer(cBuffer, logicalDevice), UniqueVulkanAllocation(cAllocation, VulkanAllocationDeleter{allocator.get()}), std::move(cAllocationInfo)};
}

vkx::Image vkx::VulkanAllocator::allocateImage(vk::Extent2D extent,
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
	if (vmaCreateImage(allocator.get(), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, &resourceImage, &resourceAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory resources.");
	}

	return vkx::Image{logicalDevice, vk::UniqueImage(resourceImage, logicalDevice), UniqueVulkanAllocation(resourceAllocation, VulkanAllocationDeleter{allocator.get()})};
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

	commandSubmitter.transitionImageLayout(resourceImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	commandSubmitter.copyBufferToImage(static_cast<vk::Buffer>(stagingBuffer), resourceImage, textureWidth, textureHeight);

	commandSubmitter.transitionImageLayout(resourceImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	stbi_image_free(pixels);
	return vkx::Image{logicalDevice, vk::UniqueImage(resourceImage, logicalDevice), UniqueVulkanAllocation(resourceAllocation, VulkanAllocationDeleter{allocator.get()})};
}

vkx::UniformBuffer vkx::VulkanAllocator::allocateUniformBuffer(std::size_t memorySize) const {
	return allocateBuffer(memorySize, vk::BufferUsageFlagBits::eUniformBuffer);
}

std::vector<vkx::UniformBuffer> vkx::VulkanAllocator::allocateUniformBuffers(std::size_t memorySize, std::size_t amount) const {
	std::vector<vkx::UniformBuffer> uniformBuffers;
	uniformBuffers.reserve(amount);
	for (auto i = 0; i < amount; i++) {
		uniformBuffers.emplace_back(allocateBuffer(memorySize, vk::BufferUsageFlagBits::eUniformBuffer));
	}
	return uniformBuffers;
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

vkx::VulkanRenderPass::operator vk::RenderPass() const {
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

float vkx::VulkanDevice::getMaxSamplerAnisotropy() const {
	return maxSamplerAnisotropy;
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

vkx::GraphicsPipeline vkx::VulkanDevice::createGraphicsPipeline(const vkx::VulkanRenderPass& renderPass, const vkx::VulkanAllocator& allocator, const vkx::GraphicsPipelineInformation& information) const {
	return vkx::GraphicsPipeline{*logicalDevice, static_cast<vk::RenderPass>(renderPass), allocator, information};
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

vkx::ArrayMesh::ArrayMesh(std::size_t vertexCount, std::size_t indexCount, const vkx::VulkanAllocator& allocator)
    : arrayVertexBuffer(allocator.allocateBuffer(vertexCount * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer)),
      arrayIndexBuffer(allocator.allocateBuffer(indexCount * sizeof(std::uint32_t), vk::BufferUsageFlagBits::eIndexBuffer)),
      arrayVertices(vertexCount),
      arrayIndices(indexCount) {

	auto giantBuffer = allocator.allocateBuffer(vertexCount * sizeof(vkx::Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
}
