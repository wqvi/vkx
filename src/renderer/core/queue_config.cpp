#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/renderer.hpp>

vkx::QueueConfig::QueueConfig(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
	const auto queueFamilies = physicalDevice.getQueueFamilyProperties();

	for (auto i = 0; i < queueFamilies.size(); i++) {
		const auto flags = queueFamilies[i].queueFlags;
		if (flags & vk::QueueFlagBits::eGraphics) {
			graphicsIndex = i;
		}

		if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
			presentIndex = i;
		}

		if (graphicsIndex && presentIndex) {
			break;
		}
	}

	if (graphicsIndex && presentIndex) {
		std::set uniqueIndices{*graphicsIndex, *presentIndex};

		std::copy(uniqueIndices.begin(), uniqueIndices.end(), std::back_inserter(indices));
	}
}

bool vkx::QueueConfig::isComplete() const {
	return graphicsIndex && presentIndex;
}

bool vkx::QueueConfig::isUniversal() const {
	return *graphicsIndex == *presentIndex;
}

std::vector<vk::DeviceQueueCreateInfo> vkx::QueueConfig::createQueueInfos(const float* queuePriorities) const {
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(indices.size());
	for (const std::uint32_t index : indices) {
		queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, index, 1, queuePriorities);
	}

	return queueCreateInfos;
}

vk::SharingMode vkx::QueueConfig::getImageSharingMode() const {
	if (isUniversal()) {
		return vk::SharingMode::eExclusive;
	}

	return vk::SharingMode::eConcurrent;
}