#pragma once

namespace vkx
{
    struct SwapchainInfo
    {
        SwapchainInfo(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface);

        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;

        [[nodiscard]] vk::SurfaceFormatKHR chooseSurfaceFormat() const;

        [[nodiscard]] vk::PresentModeKHR choosePresentMode() const;

        [[nodiscard]] vk::Extent2D chooseExtent(int width, int height) const;

        [[nodiscard]] std::uint32_t getImageCount() const;
        [[nodiscard]] bool isComplete() const;
    };
}