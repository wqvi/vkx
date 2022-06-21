#pragma once

#include <renderer/core/renderer_types.hpp>
#include <renderer/core/profile.hpp>
#include <renderer/core/vertex.hpp>
#include <renderer/core/queue_config.hpp>

namespace vkx
{
    class Device
    {
    public:
        Device() = default;

        Device(vk::UniqueInstance const &instance, vk::UniqueSurfaceKHR const &surface, Profile const &profile);

        Device(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface, Profile const &profile);

        explicit operator vk::PhysicalDevice const &() const;

        explicit operator vk::Device const &() const;

        explicit operator vk::CommandPool const &() const;

        explicit operator vk::UniqueCommandPool const &() const;

        operator vk::UniqueDevice const &() const;

        vk::Device const &operator*() const;

        vk::Device const *operator->() const;

        [[nodiscard]] std::vector<DrawCommand> createDrawCommands(std::uint32_t size) const;

        [[nodiscard]] std::uint32_t findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags const &flags) const;

        [[nodiscard]] vk::Format findSupportedFormat(std::vector<vk::Format> const &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags const &features) const;

        [[nodiscard]] vk::Format findDepthFormat() const;

        [[nodiscard]] vk::UniqueImage createImageUnique(std::uint32_t width, std::uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags const &usage) const;

        [[nodiscard]] vk::UniqueBuffer createBufferUnique(vk::DeviceSize size, vk::BufferUsageFlags const &usage) const;

        [[nodiscard]] vk::UniqueDeviceMemory allocateMemoryUnique(vk::UniqueBuffer const &buffer, vk::MemoryPropertyFlags const &flags) const;

        [[nodiscard]] vk::UniqueDeviceMemory allocateMemoryUnique(vk::UniqueImage const &image, vk::MemoryPropertyFlags const &flags) const;

        [[nodiscard]] vk::UniqueImageView createImageViewUnique(vk::Image const &image, vk::Format format, vk::ImageAspectFlags const &aspectFlags) const;

        void copyBuffer(
            vk::Buffer const &srcBuffer,
            vk::Buffer const &dstBuffer,
            vk::DeviceSize const &size) const;

        void copyBufferToImage(
            vk::Buffer const &buffer,
            vk::Image const &image,
            std::uint32_t width,
            std::uint32_t height) const;

        void transitionImageLayout(
            vk::Image const &image,
            vk::ImageLayout const &oldLayout,
            vk::ImageLayout const &newLayout) const;

        void submit(std::vector<vk::CommandBuffer> const &commandBuffers, vk::Semaphore const &waitSemaphore, vk::Semaphore const &signalSemaphore, vk::Fence const &flightFence) const;

        [[nodiscard]] vk::Result present(vk::SwapchainKHR const &swapchain, std::uint32_t imageIndex, vk::Semaphore const &signalSemaphores) const;

        [[nodiscard]] vk::UniqueImageView createTextureImageViewUnique(vk::Image const &image) const;

        [[nodiscard]] vk::UniqueSampler createTextureSamplerUnique() const;

    private:
        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice device;
        vk::UniqueCommandPool commandPool;
        vk::PhysicalDeviceProperties properties;
        
        Queues queues;

        static bool validatePhysicalDevice(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface, Profile const &profile);
    };
}