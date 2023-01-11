#pragma once

namespace vkx {
struct QueueConfig {
	std::optional<std::uint32_t> graphicsIndex{};
	std::optional<std::uint32_t> presentIndex{};
	std::vector<std::uint32_t> indices{};

	explicit QueueConfig(vk::PhysicalDevice physicalDevice,
			     vk::SurfaceKHR surface);

	[[nodiscard]] std::vector<vk::DeviceQueueCreateInfo> createQueueInfos(const float* queuePriorities) const;

	[[nodiscard]] vk::SharingMode getImageSharingMode() const;

	[[nodiscard]] bool isUniversal() const;

	[[nodiscard]] bool isComplete() const;
};
} // namespace vkx