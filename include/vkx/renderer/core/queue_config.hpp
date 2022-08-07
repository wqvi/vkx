#pragma once

#include "renderer_types.hpp"
#include <cstdint>

namespace vkx {
struct QueueConfig {
	QueueConfig(const vk::PhysicalDevice& physicalDevice, const vk::UniqueSurfaceKHR& surface);

	QueueConfig(const Device& device, const vk::UniqueSurfaceKHR& surface);

	std::uint32_t computeIndex = UINT32_MAX;
	std::uint32_t graphicsIndex = UINT32_MAX;
	std::uint32_t presentIndex = UINT32_MAX;

	std::vector<std::uint32_t> indices;

	[[nodiscard]] bool isComplete() const;

	[[nodiscard]] bool isUniversal() const;

	[[nodiscard]] std::vector<vk::DeviceQueueCreateInfo> createQueueInfos(const vk::ArrayProxy<float>& queuePriorities = {1.0f}) const;

	[[nodiscard]] vk::SharingMode getImageSharingMode() const;
};

struct Queues {
	Queues() = default;

	Queues(const Device& device, const QueueConfig& queueConfig);

	vk::Queue compute;
	vk::Queue graphics;
	vk::Queue present;
};
} // namespace vkx