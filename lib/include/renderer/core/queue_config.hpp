#pragma once

#include <renderer/core/renderer_types.hpp>

namespace vkx
{
	struct QueueConfig
	{
		QueueConfig(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface);

		QueueConfig(Device const &device, vk::UniqueSurfaceKHR const &surface);

		std::optional<std::uint32_t> computeIndex;
		std::optional<std::uint32_t> graphicsIndex;
		std::optional<std::uint32_t> presentIndex;

		std::vector<std::uint32_t> indices;

		[[nodiscard]] bool isComplete() const;

		[[nodiscard]] bool isUniversal() const;

		[[nodiscard]] std::vector<vk::DeviceQueueCreateInfo> createQueueInfos(vk::ArrayProxy<float> const &queuePriorities = {1.0f}) const;
		
		[[nodiscard]] vk::SharingMode getImageSharingMode() const;
	};

	struct Queues
	{
		Queues() = default;

		Queues(Device const &device, QueueConfig const &queueConfig);

		vk::Queue compute;
		vk::Queue graphics;
		vk::Queue present;
	};
}