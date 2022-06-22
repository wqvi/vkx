//
// Created by december on 6/21/22.
//

#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

namespace vkx {
    struct AppConfig {
        int windowWidth;
        int windowHeight;
    };

    class App {
    public:
        App() = default;

        explicit App(const AppConfig &config);

        ~App();

        void run();

    private:
        static std::uint32_t rate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char *> &extensions);

        static bool isSubset(const std::vector<const char*> &arr, const std::vector<const char *> &subset);

        SDL_Window* window = nullptr;
        VkInstance instance = nullptr;
        VkSurfaceKHR surface = nullptr;
        VkPhysicalDevice physicalDevice = nullptr;
        VkDevice logicalDevice = nullptr;
    };

    struct DeviceQueueConfig {
        DeviceQueueConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        std::optional<std::uint32_t> computeQueueIndex;
        std::optional<std::uint32_t> graphicsQueueIndex;
        std::optional<std::uint32_t> presentQueueIndex;

        [[nodiscard]] std::vector<VkDeviceQueueCreateInfo> createQueueInfos(const float *queuePriority) const;

        [[nodiscard]] bool isComplete() const;
    };

    struct SwapchainSurfaceConfig {
        SwapchainSurfaceConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        [[nodiscard]] VkSurfaceFormatKHR chooseSurfaceFormat() const;

        [[nodiscard]] VkPresentModeKHR choosePresentMode() const;

        [[nodiscard]] VkExtent2D chooseExtent(std::uint32_t width, std::uint32_t height) const;

        [[nodiscard]] std::uint32_t getImageCount() const;

        [[nodiscard]] bool isComplete() const;
    };
}