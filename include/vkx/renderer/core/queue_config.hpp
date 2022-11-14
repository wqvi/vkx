#pragma once

#include <vkx/renderer/core/renderer_types.hpp>

namespace vkx {
struct QueueConfig {
	std::optional<std::uint32_t> graphicsIndex;
	std::optional<std::uint32_t> presentIndex;
	std::vector<std::uint32_t> indices;

	explicit QueueConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	[[nodiscard]] std::vector<VkDeviceQueueCreateInfo> createQueueInfos(float queuePriorities) const;

	[[nodiscard]] VkSharingMode getImageSharingMode() const;

	[[nodiscard]] bool isUniversal() const;

	[[nodiscard]] bool isComplete() const;
};
} // namespace vkx