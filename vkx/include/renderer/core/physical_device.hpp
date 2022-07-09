//
// Created by december on 7/8/22.
//

#pragma once

#include <renderer/core/renderer_types.hpp>

namespace vkx {
    class PhysicalDevice {
    public:
        explicit PhysicalDevice(RendererContext const &rendererContext,
                                vk::UniqueSurfaceKHR const &surface,
                                vkx::Profile const &profile);

    private:
        vk::PhysicalDevice physicalDevice;
    };
}
