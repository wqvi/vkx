//
// Created by december on 7/7/22.
//

#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/core/device.hpp>

vkx::SyncObjects::SyncObjects(Device const &device)
        : imageAvailableSemaphore(device->createSemaphoreUnique({})),
          renderFinishedSemaphore(device->createSemaphoreUnique({})),
          inFlightFence(device->createFenceUnique({vk::FenceCreateFlagBits::eSignaled})) {}

std::vector<vkx::SyncObjects> vkx::SyncObjects::createSyncObjects(Device const &device) {
    std::vector<vkx::SyncObjects> objs;
    objs.resize(MAX_FRAMES_IN_FLIGHT);

    std::generate(objs.begin(), objs.end(), [&device]() { return SyncObjects{device}; });

    return objs;
}