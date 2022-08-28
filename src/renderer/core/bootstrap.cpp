#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/bootstrap.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>

#ifdef DEBUG
extern "C" {
static VkBool32 vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
		break;
	default:
		return VK_FALSE;
	}
	return VK_FALSE;
}
}
#endif

vkx::RendererBootstrap::RendererBootstrap(SDL_Window* window) {
	constexpr vk::ApplicationInfo applicationInfo{
	    "Star explorer",
	    VK_MAKE_VERSION(0, 0, 1),
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 1),
	    VK_API_VERSION_1_0};

	std::uint32_t count = 0;
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}

	std::vector<const char*> extensions;
	extensions.resize(count);
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}

#ifdef DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

#ifdef DEBUG
	constexpr std::array layers = {"VK_LAYER_KHRONOS_validation"};
#elif RELEASE
	constexpr std::array<const char*, 0> layers = {};
#endif

	const vk::InstanceCreateInfo instanceCreateInfo{
	    {},
	    &applicationInfo,
	    layers,
	    extensions};

#ifdef DEBUG
	constexpr auto messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	constexpr auto messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

	const vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
	    {},
	    messageSeverity,
	    messageType,
	    vkDebugCallback,
	    nullptr};

	vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> structureChain{
	    instanceCreateInfo,
	    debugUtilsMessengerCreateInfo};

	instance = vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
	instance = vk::createInstanceUnique(instanceCreateInfo);
#endif

	VkSurfaceKHR cSurface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, *instance, &cSurface) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}

	surface = vk::UniqueSurfaceKHR{cSurface, *instance};
}

vkx::Device vkx::RendererBootstrap::createDevice() const {
	const auto physicalDevice = getBestPhysicalDevice(*instance, *surface);
	return vkx::Device{*instance, physicalDevice, *surface};
}

vk::PhysicalDevice vkx::RendererBootstrap::getBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface) {
	if (!static_cast<bool>(instance)) {
		throw std::invalid_argument("Instance must be a valid handle.");
	}

	if (!static_cast<bool>(surface)) {
		throw std::invalid_argument("Surface must be a valid handle.");
	}

	const auto physicalDevices = instance.enumeratePhysicalDevices();

	vk::PhysicalDevice physicalDevice = nullptr;
	std::uint32_t bestRating = 0;
	for (const auto& pDevice : physicalDevices) {
		std::uint32_t rating = 0;

		const QueueConfig indices{pDevice, surface};
		if (indices.isComplete()) {
			rating++;
		}

		const SwapchainInfo info{pDevice, surface};
		if (info.isComplete()) {
			rating++;
		}

		if (pDevice.getFeatures().samplerAnisotropy) {
			rating++;
		}

		if (rating > bestRating) {
			bestRating = rating;
			physicalDevice = pDevice;
		}
	}

	if (!static_cast<bool>(physicalDevice)) {
		throw std::runtime_error("Failed to find suitable GPU to use.");
	}

	return physicalDevice;
}