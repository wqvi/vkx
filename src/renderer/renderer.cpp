#include <vkx/renderer/renderer.hpp>

/**
 * @brief This function reduces the boilerplate needed for retrieving an array from Vulkan or SDL2
 *
 * @tparam ArrayType The desired type of the returned array
 * @tparam Function The function that will be executed
 * @tparam Predicate validates if the function executed successfully
 * @tparam Parameters The paramaters being passed into the function
 * @param errorMessage The message that will be thrown if an error occurs
 * @param function The function pointer that will be executed
 * @param predicate The function that validates the function pointer return type
 * @param param The parameters passed into the function pointer
 * @return constexpr std::vector<ArrayType>
 */
template <class ArrayType, class Function, class Predicate, class... Parameters>
constexpr auto get(const char* errorMessage, Function function, Predicate predicate, Parameters... param) {
	std::uint32_t count = 0;
	auto result = function(param..., &count, nullptr);
	if (predicate(result)) {
		throw std::runtime_error(errorMessage);
	}

	std::vector<ArrayType> items{count};
	result = function(param..., &count, items.data());
	if (predicate(result)) {
		throw std::runtime_error(errorMessage);
	}

	return items;
}

/**
 * @brief This function creates a Vulkan object from provided parameters
 *
 * @tparam ObjectType The type of Vulkan object
 * @tparam Function The Vulkan function pointer to be executed
 * @tparam Predicate The function that validates the result of the vulkan function
 * @tparam Parameters The parameters passed into the Vulkan function pointer
 * @param function The Vulkan function pointer to be executed
 * @param predicate The function that validates the result of the vulkan function
 * @param param The parameters passed into the Vulkan function pointer
 * @return constexpr ObjectType
 */
template <class ObjectType, class Function, class Predicate, class... Parameters>
constexpr auto create(Function function, Predicate predicate, Parameters... param) {
	ObjectType object{};
	auto result = function(param..., &object);
	predicate(result);

	return object;
}

VkInstance vkx::createInstance(SDL_Window* const window) {
	constexpr VkApplicationInfo applicationInfo{
	    VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    nullptr,
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 1),
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 1),
	    VK_API_VERSION_1_0};

	auto instanceExtensions = get<const char*>(
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
		throw std::runtime_error("Layer not present");
	}

	if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		throw std::runtime_error("Extension not present");
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failure to create instance");
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

VkDevice vkx::createDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice) {
	const QueueConfig queueConfig{physicalDevice, surface};
	constexpr float queuePriority = 1.0f;
	const auto queueCreateInfos = queueConfig.createQueueInfos(queuePriority);

	VkPhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = true;

#ifdef DEBUG
	constexpr std::array layers = {"VK_LAYER_KHRONOS_validation"};
#elif RELEASE
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
		throw std::runtime_error("Layer not present");
	}

	if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		throw std::runtime_error("Extension not present");
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failure to create logical device.");
	}
	return device;
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

VkRenderPass vkx::createRenderPass(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Format format, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout, vk::AttachmentLoadOp loadOp) {
	const VkAttachmentDescription colorAttachment{
	    0,
	    static_cast<VkFormat>(format),
	    VK_SAMPLE_COUNT_1_BIT,
	    static_cast<VkAttachmentLoadOp>(loadOp),
	    VK_ATTACHMENT_STORE_OP_STORE,
	    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    static_cast<VkImageLayout>(initialLayout),
	    static_cast<VkImageLayout>(finalLayout)};

	const VkAttachmentReference colorAttachmentRef{
	    0,
	    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

	const VkAttachmentDescription depthAttachment{
	    0,
	    static_cast<VkFormat>(findDepthFormat(physicalDevice)),
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

	const std::array renderPassAttachments{colorAttachment, depthAttachment};

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
		throw std::runtime_error("Failed to create render pass");
	}

	return renderPass;
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