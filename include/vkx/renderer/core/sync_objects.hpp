#pragma once

namespace vkx {
struct SyncObjects {
	SyncObjects() = default;

	explicit SyncObjects(vk::Device device);

	static std::vector<SyncObjects> createSyncObjects(vk::Device device);

	void waitForFence() const;

	void resetFence() const;

	vk::Device device;
	vk::UniqueSemaphore imageAvailableSemaphore;
	vk::UniqueSemaphore renderFinishedSemaphore;
	vk::UniqueFence inFlightFence;
};
} // namespace vkx
