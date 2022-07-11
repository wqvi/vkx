//
// Created by december on 7/8/22.
//

#include <vkx/renderer/core/physical_device.hpp>
#include <vkx/renderer/core/context.hpp>

vkx::PhysicalDevice::PhysicalDevice(RendererContext const &rendererContext,
                                    vk::UniqueSurfaceKHR const &surface,
                                    vkx::Profile const &profile)
        : physicalDevice(rendererContext.getBestPhysicalDevice(surface, profile)) {

}