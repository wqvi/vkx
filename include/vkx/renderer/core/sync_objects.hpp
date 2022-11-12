#pragma once

namespace vkx {
struct SyncObjects {
	SyncObjects() = default;

	explicit SyncObjects(VkDevice device);

	static std::vector<SyncObjects> createSyncObjects(VkDevice device);

	void waitForFence() const;

	void resetFence() const;

	void destroy() const;

	VkDevice device;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
};
} // namespace vkx
