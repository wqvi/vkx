#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::QueueConfig::QueueConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	const auto queueFamilies = vkx::getArray<VkQueueFamilyProperties>(
	    vkGetPhysicalDeviceQueueFamilyProperties,
	    physicalDevice);

	for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilies.size()); i++) {
		const auto flags = queueFamilies[i].queueFlags;
		if (flags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsIndex = i;
		}

		const auto surfaceSupport = vkx::getObject<VkBool32>(
		    "Failed to get physical device surface support.",
		    vkGetPhysicalDeviceSurfaceSupportKHR,
		    [](auto a) {
			    return a != VK_SUCCESS;
		    },
		    physicalDevice, i, surface);

		if (surfaceSupport) {
			presentIndex = i;
		}

		if (isComplete()) {
			break;
		}
	}

	if (isComplete()) {
		std::set uniqueIndices{*graphicsIndex, *presentIndex};

		std::copy(uniqueIndices.begin(), uniqueIndices.end(), std::back_inserter(indices));
	}
}

bool vkx::QueueConfig::isComplete() const {
	return graphicsIndex.has_value() && presentIndex.has_value();
}

bool vkx::QueueConfig::isUniversal() const {
	return *graphicsIndex == *presentIndex;
}

std::vector<VkDeviceQueueCreateInfo> vkx::QueueConfig::createQueueInfos(float queuePriorities) const {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (const std::uint32_t index : indices) {
		const VkDeviceQueueCreateInfo queueCreateInfo{
		    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		    nullptr,
		    0,
		    index,
		    1,
		    &queuePriorities};

		queueCreateInfos.push_back(queueCreateInfo);
	}

	return queueCreateInfos;
}

VkSharingMode vkx::QueueConfig::getImageSharingMode() const {
	if (isUniversal()) {
		return VK_SHARING_MODE_EXCLUSIVE;
	}

	return VK_SHARING_MODE_CONCURRENT;
}