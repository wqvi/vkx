#pragma once

namespace vkx {
struct SyncObjects {
	vk::Device logicalDevice = nullptr;
	vk::UniqueSemaphore imageAvailableSemaphore;
	vk::UniqueSemaphore renderFinishedSemaphore;
	vk::UniqueFence inFlightFence;

	SyncObjects() = default;

	explicit SyncObjects(vk::Device logicalDevice);

	void waitForFence() const;

	void resetFence() const;
};
} // namespace vkx
