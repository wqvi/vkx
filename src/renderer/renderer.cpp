#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/renderer.hpp>
#include <vulkan/vulkan_handles.hpp>

vk::UniqueInstance vkx::createInstance(SDL_Window* const window) {
	constexpr vk::ApplicationInfo applicationInfo{"VKX", VK_MAKE_VERSION(0, 0, 1), "VKX", VK_MAKE_VERSION(0, 0, 1), VK_API_VERSION_1_0};

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

	const vk::InstanceCreateInfo instanceCreateInfo{{}, &applicationInfo, instanceLayers, instanceExtensions};

#ifdef DEBUG
	constexpr auto messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	constexpr auto messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

	constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{{}, messageSeverity, messageType, [](auto, auto, const auto* pCallbackData, auto*) { SDL_Log("%s", pCallbackData->pMessage); return VK_FALSE; }, nullptr};

	const vk::StructureChain structureChain{instanceCreateInfo, debugUtilsMessengerCreateInfo};

	return vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
	return vk::createInstanceUnique(instanceCreateInfo);
#endif
}

vk::UniqueSurfaceKHR vkx::createSurface(SDL_Window* const window, vk::Instance instance) {
	VkSurfaceKHR cSurface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, instance, &cSurface) != SDL_TRUE) {
		throw std::runtime_error("Failed to create vulkan surface.");
	}
	return vk::UniqueSurfaceKHR{cSurface, instance};
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