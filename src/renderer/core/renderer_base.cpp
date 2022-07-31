#include <SDL2/SDL_error.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_vulkan.h>
#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <vkx/renderer/core/renderer_base.hpp>

#include <iostream>
#include <vkx/debug.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/uniform_buffer.hpp>
#include <vkx/vkx_exceptions.hpp>
#include <vulkan/vulkan_core.h>

static constexpr std::uint32_t API_VERSION = VK_API_VERSION_1_0;

SwapchainInfo::SwapchainInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	std::uint32_t count = 0;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to get physical device surface format count.");
	}

	formats.resize(count);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to get physical device surface formats.");
	}

	count = 0;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to get physical device surface present mode count.");
	}

	presentModes.resize(count);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, presentModes.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to get physical device surface present modes.");
	}
}

VkSurfaceFormatKHR SwapchainInfo::chooseSurfaceFormat() const {
	for (const auto& surfaceFormat : formats) {
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return surfaceFormat;
		}
	}

	return formats[0];
}

VkPresentModeKHR SwapchainInfo::choosePresentMode() const {
	for (VkPresentModeKHR presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapchainInfo::chooseExtent(int width, int height) const {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}

	VkExtent2D extent = {
	    .width = static_cast<std::uint32_t>(width),
	    .height = static_cast<std::uint32_t>(height)};

	extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
}

bool SwapchainInfo::complete() const noexcept {
	return !formats.empty() && !presentModes.empty();
}

static bool isSubset(const std::vector<const char*>& arr, const std::vector<const char*>& subset) {
	auto iter = arr.begin();
	const auto end = arr.end();

	if (arr.size() < subset.size()) {
		throw std::invalid_argument("Arr must be larger than subset.");
	}

	for (const char* subsetStr : subset) {
		for (iter = arr.begin(); iter != end; iter++) {
			if (std::strcmp(*iter, subsetStr) == 0)
				break;
		}

		if (iter == end) {
			return false;
		}
	}

	return true;
}

QueueConfig::QueueConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	std::uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies;
	queueFamilies.resize(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilies.data());

	for (std::uint32_t i = 0; i < count; i++) {
		auto flags = queueFamilies[i].queueFlags;
		if (flags & VK_QUEUE_GRAPHICS_BIT) {
			if (graphicsIndex == UINT32_MAX) {
				graphicsIndex = i;
			}
		}

		VkBool32 support = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &support);
		if (support) {
			if (presentIndex == UINT32_MAX) {
				presentIndex = i;
			}
		}

		if (complete()) {
			break;
		}
	}
}

std::vector<VkDeviceQueueCreateInfo> QueueConfig::createQueueInfos(float priority) const {
	std::set<std::uint32_t> uniqueIndices{
	    graphicsIndex,
	    presentIndex};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.resize(uniqueIndices.size());
	for (const std::uint32_t index : uniqueIndices) {
		VkDeviceQueueCreateInfo queueCreateInfo = {
		    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		    .pNext = nullptr,
		    .flags = 0,
		    .queueFamilyIndex = index,
		    .queueCount = 1,
		    .pQueuePriorities = &priority};

		queueCreateInfos.push_back(queueCreateInfo);
	}

	return queueCreateInfos;
}

bool QueueConfig::complete() const noexcept {
	return graphicsIndex != UINT32_MAX && presentIndex != UINT32_MAX;
}

VulkanDevice::VulkanDevice(VkInstance instance, VkSurfaceKHR surface) {
	physicalDevice = pickPhysicalDevice(instance, surface);

	QueueConfig queueConfig{physicalDevice, surface};

	device = createDevice(queueConfig, physicalDevice, surface);
	commandPool = createCommandPool(queueConfig, device);
}

void VulkanDevice::destroy() const noexcept {
	// vmaDestroyAllocator(allocator);
	// vkDestroyCommandPool(device, commandPool, nullptr);
	// vkDestroyDevice(device, nullptr);
	SDL_Log("Destroyed VulkanDevice.");
}

VkPhysicalDevice VulkanDevice::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
	std::uint32_t count = 0;
	if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to get count of available physical devices.");
	}

	std::vector<VkPhysicalDevice> physicalDevices;
	physicalDevices.resize(count);
	if (vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to enumerate physical devices.");
	}

	VkPhysicalDevice bestPhysicalDevice = nullptr;
	for (VkPhysicalDevice physicalDevice : physicalDevices) {
		std::uint32_t rating = 0;

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		count = 0;
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to get count of available device extension properties.");
		}

		std::vector<VkExtensionProperties> extensions;
		extensions.resize(count);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to enumerate device extension properties.");
		}

		bool valid = false;
		for (const VkExtensionProperties& ext : extensions) {
			if (std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
				valid = true;
			}

			if (valid) {
				break;
			}
		}

		if (!valid) {
			continue;
		}

		QueueConfig queueConfig{physicalDevice, surface};
		if (!queueConfig.complete()) {
			continue;
		}

		SwapchainInfo swapchainInfo{physicalDevice, surface};
		if (!swapchainInfo.complete()) {
			continue;
		}

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);
		if (!features.samplerAnisotropy) {
			continue;
		}

		bestPhysicalDevice = physicalDevice;
		break;
	}

	if (bestPhysicalDevice == nullptr) {
		throw std::runtime_error("Failed to pick physical device.");
	}

	return bestPhysicalDevice;
}

VkDevice VulkanDevice::createDevice(const QueueConfig& queueConfig, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	float queuePriority = 1.0f;
	const auto queueConfigs = queueConfig.createQueueInfos(queuePriority);

	VkPhysicalDeviceFeatures requestedFeatures = {};
	requestedFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = static_cast<std::uint32_t>(queueConfigs.size()),
		.pQueueCreateInfos = queueConfigs.data(),
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
		.pEnabledFeatures = &requestedFeatures
	};

	VkDevice device = nullptr;
	auto result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
	if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		throw std::runtime_error("Extension not present upon device creation.");
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create device.");
	}

	return device;
}

VkCommandPool VulkanDevice::createCommandPool(const QueueConfig& queueConfig, VkDevice device) {
	VkCommandPoolCreateInfo commandPoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueConfig.graphicsIndex
	};

	VkCommandPool commandPool = nullptr;
	if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create device command pool.");
	}

	return commandPool;
}

VulkanBootstrap::VulkanBootstrap(SDL_Window* window) {
	VkApplicationInfo applicationInfo = {
	    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    .pNext = nullptr,
	    .pApplicationName = "jewelry",
	    .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
	    .pEngineName = "vulcan",
	    .engineVersion = VK_MAKE_VERSION(0, 0, 0),
	    .apiVersion = API_VERSION};

	instance = initInstance(window, &applicationInfo);
	surface = initSurface(window, instance);

	device = VulkanDevice{instance, surface};
}

VulkanBootstrap::~VulkanBootstrap() {
	device.destroy();
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	SDL_Log("Destroyed VulkanBootstrap.");
}

VkBool32 VulkanBootstrap::debug(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
	} else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
	} else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
	}
	return VK_FALSE;
}

VkInstance VulkanBootstrap::initInstance(SDL_Window* window, VkApplicationInfo* applicationInfo) {
	std::uint32_t count = 0;
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}
	std::vector<const char*> extensions(count);
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}

#ifdef DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

#ifdef DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	    .pNext = nullptr,
	    .flags = 0,
	    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	    .pfnUserCallback = debug,
	    .pUserData = nullptr};
#endif

#ifdef DEBUG
	const char* layer = "VK_LAYER_KHRONOS_validation";
#endif

	VkInstanceCreateInfo instanceCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#ifdef DEBUG
	    .pNext = &debugUtilsMessengerCreateInfo,
#else
	    .pNext = nullptr,
#endif
	    .flags = 0,
	    .pApplicationInfo = applicationInfo,
#ifdef DEBUG
	    .enabledLayerCount = 1,
	    .ppEnabledLayerNames = &layer,
#else
	    .enabledLayerCount = 0,
	    .ppEnabledLayerNames = nullptr,
#endif
	    .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
	    .ppEnabledExtensionNames = extensions.data()};

	VkInstance instance = nullptr;
	auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	switch (result) {
	case VK_SUCCESS:
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		throw std::runtime_error("Layer not present upon instance creation.");
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		throw std::runtime_error("Extension not present upon instance creation.");
	default:
		throw std::runtime_error("Failed to create instance.");
	}

	return instance;
}

VkSurfaceKHR VulkanBootstrap::initSurface(SDL_Window* window, VkInstance instance) {
	VkSurfaceKHR surface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE) {
		return nullptr;
	}
	return surface;
}

vkx::RendererBase::RendererBase(SDL_Window* window) : window(window) {
	static constexpr vk::ApplicationInfo applicationInfo{
	    "Jewelry", VK_MAKE_VERSION(0, 0, 1), "Vulcan", VK_MAKE_VERSION(0, 0, 1),
	    VK_API_VERSION_1_0};

	std::uint32_t count = 0;
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr) != SDL_TRUE) {
		throw vkx::SDLError();
	}
	std::vector<const char*> extensions(count);
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()) !=
	    SDL_TRUE) {
		throw vkx::SDLError();
	}

#ifdef DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	std::vector<const char*> layers{
#ifdef DEBUG
	    "VK_LAYER_KHRONOS_validation",
#endif
	};

#ifdef DEBUG
	auto availableLayers = vk::enumerateInstanceLayerProperties();
	std::vector<const char*> currentStrLayers;
	std::transform(availableLayers.begin(), availableLayers.end(),
		       std::back_inserter(currentStrLayers),
		       [](const auto& props) { return props.layerName; });
	if (!isSubset(currentStrLayers, layers)) {
		throw vkx::VulkanError("Failed to find requested Vulkan instance layers.");
	}
#endif

	vk::InstanceCreateInfo instanceCreateInfo{
	    {}, &applicationInfo, layers, extensions};

#ifdef DEBUG
	auto messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			       vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			       vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	auto messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			   vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			   vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
	    {}, messageSeverity, messageType, vkDebugCallback, nullptr};

	vk::StructureChain structureChain{instanceCreateInfo,
					  debugUtilsMessengerCreateInfo};

	instance =
	    vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
	instance = vk::createInstanceUnique(instanceCreateInfo);
#endif

	VkSurfaceKHR cSurface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, *instance, &cSurface) != SDL_TRUE) {
		throw vkx::SDLError();
	}
	surface = vk::UniqueSurfaceKHR(cSurface, *instance);

	auto physicalDevices = instance->enumeratePhysicalDevices();
	vk::PhysicalDevice bestPhysicalDevice = nullptr;
	std::uint32_t bestRating = 0;
	for (const auto& pDevice : physicalDevices) {
		std::uint32_t rating = 0;

		auto extensionProperties = pDevice.enumerateDeviceExtensionProperties();
		std::vector<const char*> stringExtensions;
		std::transform(extensionProperties.begin(), extensionProperties.end(),
			       std::back_inserter(stringExtensions),
			       [](const auto& props) { return props.extensionName; });
		if (isSubset(stringExtensions, extensions)) {
			rating++;
		}

		if (QueueConfig indices{pDevice, surface}; indices.isComplete()) {
			rating++;
		}

		if (SwapchainInfo info{pDevice, surface}; info.isComplete()) {
			rating++;
		}

		if (pDevice.getFeatures().samplerAnisotropy) {
			rating++;
		}

		if (rating > bestRating) {
			bestRating = rating;
			bestPhysicalDevice = pDevice;
		}
	}

	if (!static_cast<bool>(bestPhysicalDevice)) {
		throw vkx::VulkanError("Failure to initialize device.");
	}

	device = std::make_unique<vkx::Device>(instance, bestPhysicalDevice, surface);

	createSwapchain();

	vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0, vk::DescriptorType::eUniformBuffer, 1,
	    vk::ShaderStageFlagBits::eVertex, nullptr};

	vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1, vk::DescriptorType::eCombinedImageSampler, 1,
	    vk::ShaderStageFlagBits::eFragment, nullptr};

	vk::DescriptorSetLayoutBinding lightLayoutBinding{
	    2, vk::DescriptorType::eUniformBuffer, 1,
	    vk::ShaderStageFlagBits::eFragment, nullptr};

	vk::DescriptorSetLayoutBinding materialLayoutBinding{
	    3, vk::DescriptorType::eUniformBuffer, 1,
	    vk::ShaderStageFlagBits::eFragment, nullptr};

	std::vector<vk::DescriptorSetLayoutBinding> bindings{
	    uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding,
	    materialLayoutBinding};

	vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};

	descriptorSetLayout = (*device)->createDescriptorSetLayoutUnique(layoutInfo);

	graphicsPipeline = GraphicsPipeline{*device, swapchain.extent, renderPass,
					    descriptorSetLayout};

	drawCommands = device->createDrawCommands(MAX_FRAMES_IN_FLIGHT);

	syncObjects = SyncObjects::createSyncObjects(*device);

	createDescriptorPool();
}

namespace vkx {
void RendererBase::recreateSwapchain() {
	int width, height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);
	while (width == 0 || height == 0) {
		SDL_Vulkan_GetDrawableSize(window, &width, &height);
		SDL_WaitEvent(nullptr);
	}

	(*device)->waitIdle();

	createSwapchain();
	graphicsPipeline = GraphicsPipeline{*device, swapchain.extent, renderPass,
					    descriptorSetLayout};
}

void RendererBase::createDescriptorPool() {
	vk::DescriptorPoolSize uniformBufferDescriptor{
	    vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT};

	std::array<vk::DescriptorPoolSize, 4> poolSizes{};
	std::fill(poolSizes.begin(), poolSizes.end(), uniformBufferDescriptor);

	poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;

	vk::DescriptorPoolCreateInfo poolInfo{{}, MAX_FRAMES_IN_FLIGHT, poolSizes};

	descriptorPool = (*device)->createDescriptorPoolUnique(poolInfo);
}

void RendererBase::createDescriptorSets(
    const std::vector<UniformBuffer<MVP>>& mvpBuffers,
    const std::vector<UniformBuffer<DirectionalLight>>& lightBuffers,
    const std::vector<UniformBuffer<Material>>& materialBuffers,
    const Texture& texture) {
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
						     *descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo{*descriptorPool, layouts};

	descriptorSets = (*device)->allocateDescriptorSets(allocInfo);

	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		std::array<vk::WriteDescriptorSet, 4> descriptorWrites{
		    mvpBuffers[i].createWriteDescriptorSet(descriptorSets[i], 0),
		    texture.createWriteDescriptorSet(descriptorSets[i], 1),
		    lightBuffers[i].createWriteDescriptorSet(descriptorSets[i], 2),
		    materialBuffers[i].createWriteDescriptorSet(descriptorSets[i], 3),
		};

		(*device)->updateDescriptorSets(descriptorWrites, {});
	}
}

void RendererBase::drawFrame(const UniformBuffer<MVP>& mvpBuffer,
			     const UniformBuffer<DirectionalLight>& lightBuffer,
			     const UniformBuffer<Material>& materialBuffer,
			     const VertexBuffer& vertexBuffer,
			     const IndexBuffer& indexBuffer,
			     std::uint32_t indexCount,
			     std::uint32_t& currentIndexFrame) {
	static_cast<void>((*device)->waitForFences(
	    *syncObjects[currentIndexFrame].inFlightFence, true, UINT64_MAX));
	auto [result, imageIndex] = swapchain.acquireNextImage(
	    *device, syncObjects[currentIndexFrame].imageAvailableSemaphore);

	if (result == vk::Result::eErrorOutOfDateKHR) {
		recreateSwapchain();
		return;
	} else if (result != vk::Result::eSuccess &&
		   result != vk::Result::eSuboptimalKHR) {
		throw vkx::VulkanError(result);
	}

	mvpBuffer.mapMemory();
	lightBuffer.mapMemory();
	materialBuffer.mapMemory();

	(*device)->resetFences(*syncObjects[currentIndexFrame].inFlightFence);

	drawCommands[currentIndexFrame].record(
	    *renderPass, *swapchain.framebuffers[imageIndex], swapchain.extent,
	    *graphicsPipeline.pipeline, *graphicsPipeline.layout,
	    descriptorSets[currentIndexFrame], vertexBuffer, indexBuffer, indexCount);

	std::vector<vk::CommandBuffer> commandBuffers{
	    static_cast<vk::CommandBuffer>(drawCommands[currentIndexFrame])};
	device->submit(commandBuffers,
		       *syncObjects[currentIndexFrame].imageAvailableSemaphore,
		       *syncObjects[currentIndexFrame].renderFinishedSemaphore,
		       *syncObjects[currentIndexFrame].inFlightFence);

	result =
	    device->present(swapchain, imageIndex,
			    *syncObjects[currentIndexFrame].renderFinishedSemaphore);

	if (result == vk::Result::eErrorOutOfDateKHR ||
	    result == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapchain();
	} else if (result != vk::Result::eSuccess) {
		throw vkx::VulkanError(result);
	}

	currentIndexFrame = (currentIndexFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

std::uint32_t RendererBase::getCurrentFrameIndex() const {
	return currentFrame;
}

void RendererBase::createSwapchain() {
	swapchain = vkx::Swapchain{*device, surface, window, swapchain};

	renderPass = createRenderPass();

	swapchain.createFramebuffers(*device, renderPass);
}

vk::UniqueRenderPass
RendererBase::createRenderPass(vk::AttachmentLoadOp loadOp) const {
	vk::AttachmentDescription colorAttachment{
	    {},				      // flags
	    swapchain.imageFormat,	      // format
	    vk::SampleCountFlagBits::e1,      // samples
	    loadOp,			      // loadOp
	    vk::AttachmentStoreOp::eStore,    // storeOp
	    vk::AttachmentLoadOp::eDontCare,  // stencilLoadOp
	    vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
	    vk::ImageLayout::eUndefined,      // initialLayout
	    vk::ImageLayout::ePresentSrcKHR   // finalLayout
	};

	vk::AttachmentReference colorAttachmentRef{
	    0,					     // attachment
	    vk::ImageLayout::eColorAttachmentOptimal // layout
	};

	vk::AttachmentDescription depthAttachment{
	    {},						    // flags
	    device->findDepthFormat(),			    // format
	    vk::SampleCountFlagBits::e1,		    // samples
	    vk::AttachmentLoadOp::eClear,		    // loadOp
	    vk::AttachmentStoreOp::eDontCare,		    // storeOp
	    vk::AttachmentLoadOp::eDontCare,		    // stencilLoadOp
	    vk::AttachmentStoreOp::eDontCare,		    // stencilStoreOp
	    vk::ImageLayout::eUndefined,		    // initialLayout
	    vk::ImageLayout::eDepthStencilAttachmentOptimal // finalLayout
	};

	vk::AttachmentReference depthAttachmentRef{
	    1,						    // attachment
	    vk::ImageLayout::eDepthStencilAttachmentOptimal // layout
	};

	vk::SubpassDescription subpass{
	    {},				      // flags
	    vk::PipelineBindPoint::eGraphics, // pipelineBindPoint
	    {},				      // inputAttachments
	    colorAttachmentRef,		      // colorAttachments
	    {},				      // resolveAttachments
	    &depthAttachmentRef,	      // pDepthStencilAttachment
	    {}				      // preserveAttachments
	};

	auto dependencyStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
				   vk::PipelineStageFlagBits::eEarlyFragmentTests;
	auto dependencyAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
				    vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	vk::SubpassDependency dependency{
	    VK_SUBPASS_EXTERNAL, // srcSubpass
	    0,			 // dstSubpass
	    dependencyStageMask, // srcStageMask
	    dependencyStageMask, // dstStageMask
	    {},			 // srcAccessMask
	    dependencyAccessMask // dstAccessMask
	};

	std::vector renderPassAttachments{colorAttachment, depthAttachment};

	vk::RenderPassCreateInfo renderPassInfo{
	    {},			   // flags
	    renderPassAttachments, // attachments
	    subpass,		   // subpasses
	    dependency		   // dependencies
	};

	return (*device)->createRenderPassUnique(renderPassInfo);
}
} // namespace vkx

vkx::Mesh vkx::RendererBase::allocateMesh(
    const std::vector<Vertex>& vertices,
    const std::vector<std::uint32_t>& indices) const {
	return vkx::Mesh{vertices, indices, *device};
}

vkx::Texture
vkx::RendererBase::allocateTexture(const std::string& textureFile) const {
	return vkx::Texture{textureFile, *device};
}

void vkx::RendererBase::waitIdle() const { (*device)->waitIdle(); }
