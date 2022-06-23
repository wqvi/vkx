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

    static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

    static std::uint32_t findMemoryType(VkPhysicalDevice physicalDevice, std::uint32_t typeFilter, VkMemoryPropertyFlags flags);

    static VkImage createImage(VkDevice logicalDevice, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);

    static VkDeviceMemory allocateMemory(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags flags);

    static VkImageView createImageView(VkDevice logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    class SwapChain {
    public:
        SwapChain() = default;

        SwapChain(std::nullptr_t);

        explicit SwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, int windowWidth, int windowHeight, const SwapChain &oldSwapChain);

        SwapChain(const SwapChain &) = delete;

        SwapChain(SwapChain &&) = default;

        ~SwapChain();

        SwapChain & operator=(const SwapChain&) = delete;

        SwapChain & operator=(SwapChain&&) = default;

        void createFramebuffers(VkRenderPass renderPass);

        VkResult acquireNextImage(VkDevice logicalDevice, VkSemaphore imageAvailableSemaphore, std::uint32_t &imageIndex) const;

    private:
        // Keep this for deleting the swapChain
        VkDevice logicalDevice = nullptr;

        VkSwapchainKHR swapchain = nullptr;
        VkFormat imageFormat;
        VkExtent2D extent;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;

        VkImage depthImage = nullptr;
        VkDeviceMemory depthImageMemory = nullptr;
        VkImageView depthImageView = nullptr;

        std::vector<VkFramebuffer> framebuffers;
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

        SwapChain swapchain = nullptr;
    };

    struct DeviceQueueConfig {
        DeviceQueueConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        std::optional<std::uint32_t> computeQueueIndex;
        std::optional<std::uint32_t> graphicsQueueIndex;
        std::optional<std::uint32_t> presentQueueIndex;

        [[nodiscard]] std::vector<std::uint32_t> getContiguousIndices() const;

        [[nodiscard]] std::vector<VkDeviceQueueCreateInfo> createQueueInfos(const float *queuePriority) const;

        [[nodiscard]] bool isUniversal() const;

        [[nodiscard]] VkSharingMode getImageSharingMode() const;

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