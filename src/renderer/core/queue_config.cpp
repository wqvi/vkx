#include <cstdint>
#include <vkx/renderer/core/queue_config.hpp>

#include <vkx/renderer/core/device.hpp>

namespace vkx {
QueueConfig::QueueConfig(const vk::PhysicalDevice& physicalDevice, const vk::UniqueSurfaceKHR& surface) {
	const auto queueFamilies = physicalDevice.getQueueFamilyProperties();

	for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilies.size()); i++) {
		auto flags = queueFamilies[i].queueFlags;
		if (flags & vk::QueueFlagBits::eCompute) {
			if (computeIndex == UINT32_MAX) {
				computeIndex = i;
			}
		}

		if (flags & vk::QueueFlagBits::eGraphics) {
			if (graphicsIndex == UINT32_MAX) {
				graphicsIndex = i;
			}
		}

		if (physicalDevice.getSurfaceSupportKHR(i, *surface)) {
			if (presentIndex == UINT32_MAX) {
				presentIndex = i;
			}
		}

		if (isComplete()) {
			break;
		}
	}

	if (isUniversal() && isComplete()) {
		std::set<std::uint32_t> uniqueIndices{
		    graphicsIndex,
		    computeIndex,
		    presentIndex};

		std::copy(uniqueIndices.begin(), uniqueIndices.end(), std::back_inserter(indices));
	}
}

QueueConfig::QueueConfig(const Device& device, const vk::UniqueSurfaceKHR& surface)
    : QueueConfig(static_cast<vk::PhysicalDevice>(device), surface) {}

bool QueueConfig::isComplete() const {
	return graphicsIndex != UINT32_MAX && computeIndex != UINT32_MAX && presentIndex != UINT32_MAX;
}

bool QueueConfig::isUniversal() const {
	return computeIndex == graphicsIndex && graphicsIndex == presentIndex;
}

std::vector<vk::DeviceQueueCreateInfo> QueueConfig::createQueueInfos(const vk::ArrayProxy<float>& queuePriorities) const {
	if (queuePriorities.size() <= 0) {
		throw std::invalid_argument("The queuePriorities parameter's size must not be zero or less.");
	}

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	for (std::uint32_t index : indices) {
		vk::DeviceQueueCreateInfo queueCreateInfo{
		    {},			    // flags
		    index,		    // queueFamilyIndex
		    queuePriorities.size(), // queueCount
		    queuePriorities.data()  // pQueuePriorities
		};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	return queueCreateInfos;
}

vk::SharingMode QueueConfig::getImageSharingMode() const {
	if (isUniversal()) {
		return vk::SharingMode::eExclusive;
	}
	return vk::SharingMode::eConcurrent;
}

Queues::Queues(const Device& device, const QueueConfig& queueConfig)
    : compute(device->getQueue(queueConfig.computeIndex, 0)),
      graphics(device->getQueue(queueConfig.graphicsIndex, 0)),
      present(device->getQueue(queueConfig.presentIndex, 0)) {}
} // namespace vkx