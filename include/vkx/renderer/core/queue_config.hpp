#pragma once

namespace vkx {
struct QueueConfig {
	std::optional<std::uint32_t> graphicsIndex;
	std::optional<std::uint32_t> presentIndex;
	std::vector<std::uint32_t> indices;

	QueueConfig(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

	[[nodiscard]] bool isComplete() const;

	[[nodiscard]] bool isUniversal() const;

	[[nodiscard]] std::vector<vk::DeviceQueueCreateInfo> createQueueInfos(float queuePriorities) const;

	[[nodiscard]] vk::SharingMode getImageSharingMode() const;
};
} // namespace vkx