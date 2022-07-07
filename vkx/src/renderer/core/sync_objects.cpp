//
// Created by december on 7/7/22.
//

#include <renderer/core/sync_objects.hpp>
#include <renderer/core/device.hpp>

vkx::SyncObjects::SyncObjects(Device const &device)
        : imageAvailableSemaphore(device->createSemaphoreUnique({})),
          renderFinishedSemaphore(device->createSemaphoreUnique({})),
          inFlightFence(device->createFenceUnique({vk::FenceCreateFlagBits::eSignaled})) {}

std::vector<vkx::SyncObjects> vkx::SyncObjects::createSyncObjects(Device const &device) {
    std::vector<vkx::SyncObjects> objs;
    objs.resize(MAX_FRAMES_IN_FLIGHT);

    std::ranges::generate(objs, [&device]() { return SyncObjects{device}; });

    return objs;
}