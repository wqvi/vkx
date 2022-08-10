#pragma once

#include "renderer_types.hpp"
#include <cstdint>
#include <optional>

namespace vkx {
struct QueueConfig {
	QueueConfig(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

	QueueConfig(const Device& device, vk::SurfaceKHR surface);

	std::optional<std::uint32_t> graphicsIndex;
	std::optional<std::uint32_t> presentIndex;

	std::vector<std::uint32_t> indices;

	[[nodiscard]] bool isComplete() const;

	[[nodiscard]] bool isUniversal() const;

	[[nodiscard]] std::vector<vk::DeviceQueueCreateInfo> createQueueInfos(float queuePriorities) const;

	[[nodiscard]] vk::SharingMode getImageSharingMode() const;
};

struct Queues {
	Queues() = default;

	Queues(const Device& device, const QueueConfig& queueConfig);

	vk::Queue graphics;
	vk::Queue present;
};
} // namespace vkx