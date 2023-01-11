#include <vkx/renderer/sync_objects.hpp>

vkx::SyncObjects::SyncObjects(vk::Device logicalDevice) 
	: logicalDevice(logicalDevice) {
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo{};

	constexpr vk::FenceCreateInfo fenceCreateInfo{vk::FenceCreateFlagBits::eSignaled};

	imageAvailableSemaphore = logicalDevice.createSemaphoreUnique(semaphoreCreateInfo);
	renderFinishedSemaphore = logicalDevice.createSemaphoreUnique(semaphoreCreateInfo);
	inFlightFence = logicalDevice.createFenceUnique(fenceCreateInfo);
}

void vkx::SyncObjects::waitForFence() const {
	static_cast<void>(logicalDevice.waitForFences(*inFlightFence, true, UINT64_MAX));
}

void vkx::SyncObjects::resetFence() const {
	logicalDevice.resetFences(*inFlightFence);
}