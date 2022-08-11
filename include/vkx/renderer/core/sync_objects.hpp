#pragma once

#include "renderer_types.hpp"

namespace vkx {
    struct SyncObjects {
        SyncObjects() = default;

        explicit SyncObjects(vk::Device device);

        static std::vector<SyncObjects> createSyncObjects(vk::Device device);

        void waitForFence() const;

        vk::Device device;
        vk::UniqueSemaphore imageAvailableSemaphore;
        vk::UniqueSemaphore renderFinishedSemaphore;
        vk::UniqueFence inFlightFence;
    };
}
