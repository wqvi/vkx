//
// Created by december on 7/7/22.
//

#pragma once

#include <renderer/core/renderer_types.hpp>

namespace vkx {
    struct SyncObjects {
        SyncObjects() = default;

        explicit SyncObjects(Device const &device);

        static std::vector<SyncObjects> createSyncObjects(Device const &device);

        vk::UniqueSemaphore imageAvailableSemaphore;
        vk::UniqueSemaphore renderFinishedSemaphore;
        vk::UniqueFence inFlightFence;
    };
}
