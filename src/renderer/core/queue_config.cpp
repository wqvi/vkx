#include <cstdint>
#include <vkx/renderer/core/queue_config.hpp>

#include <vkx/renderer/core/device.hpp>

vkx::QueueConfig::QueueConfig(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
	const auto queueFamilies = physicalDevice.getQueueFamilyProperties();

	for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilies.size()); i++) {
		const auto flags = queueFamilies[i].queueFlags;
		if (flags & vk::QueueFlagBits::eGraphics) {
			graphicsIndex = i;
		}

		if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
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

vkx::QueueConfig::QueueConfig(const Device& device, vk::SurfaceKHR surface)
    : QueueConfig(static_cast<vk::PhysicalDevice>(device), surface) {}

bool vkx::QueueConfig::isComplete() const {
	return graphicsIndex.has_value() && presentIndex.has_value();
}

bool vkx::QueueConfig::isUniversal() const {
	return *graphicsIndex == *presentIndex;
}

std::vector<vk::DeviceQueueCreateInfo> vkx::QueueConfig::createQueueInfos(float queuePriorities) const {
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	for (const std::uint32_t index : indices) {
		const vk::DeviceQueueCreateInfo queueCreateInfo(
		    {},
		    index,
		    1,
		    &queuePriorities);
		queueCreateInfos.push_back(queueCreateInfo);
	}

	return queueCreateInfos;
}

vk::SharingMode vkx::QueueConfig::getImageSharingMode() const {
	if (isUniversal()) {
		return vk::SharingMode::eExclusive;
	}
	return vk::SharingMode::eConcurrent;
}

vkx::Queues::Queues(const Device& device, const QueueConfig& queueConfig)
    : graphics(device->getQueue(*queueConfig.graphicsIndex, 0)),
      present(device->getQueue(*queueConfig.presentIndex, 0)) {}