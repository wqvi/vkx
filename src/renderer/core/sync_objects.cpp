//
// Created by december on 7/7/22.
//

#include <cstdint>
#include <vkx/renderer/core/device.hpp>
#include <vkx/renderer/core/sync_objects.hpp>

vkx::SyncObjects::SyncObjects(vk::Device device)
    : device(device),
      imageAvailableSemaphore(device.createSemaphoreUnique({})),
      renderFinishedSemaphore(device.createSemaphoreUnique({})),
      inFlightFence(device.createFenceUnique({vk::FenceCreateFlagBits::eSignaled})) {}

std::vector<vkx::SyncObjects> vkx::SyncObjects::createSyncObjects(vk::Device device) {
	std::vector<vkx::SyncObjects> objs;
	objs.resize(MAX_FRAMES_IN_FLIGHT);

	std::generate(objs.begin(), objs.end(), [&device]() { return SyncObjects{device}; });

	return objs;
}

void vkx::SyncObjects::waitForFence() const {
	static_cast<void>(device.waitForFences(*inFlightFence, true, UINT64_MAX));
}