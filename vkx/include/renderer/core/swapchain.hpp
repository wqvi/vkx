#pragma once

#include <window.hpp>
#include <renderer/core/device.hpp>

namespace vkx
{
    class Swapchain
    {
    public:
        Swapchain();

        Swapchain(
            Device const &device,
            vk::UniqueSurfaceKHR const &surface,
            SDL_Window *window,
            Swapchain const &oldSwapchain);

        Swapchain(const Device &device,
                  const vk::UniqueSurfaceKHR &surface,
                  const SDLWindow &window,
                  const Swapchain &oldSwapchain);

        operator vk::SwapchainKHR const &() const;

        operator vk::UniqueSwapchainKHR const &() const;

        void createFramebuffers(Device const &device, vk::UniqueRenderPass const &renderPass);

        vk::ResultValue<std::uint32_t> acquireNextImage(Device const &device, vk::UniqueSemaphore const &semaphore) const;
        
        vk::UniqueSwapchainKHR swapchain;
        vk::Format imageFormat;
        vk::Extent2D extent;
        std::vector<vk::Image> images;
        std::vector<vk::UniqueImageView> imageViews;

        vk::UniqueImage depthImage;
		vk::UniqueDeviceMemory depthImageMemory;
		vk::UniqueImageView depthImageView;

        std::vector<vk::UniqueFramebuffer> framebuffers;
    };
}