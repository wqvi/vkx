#include <vkx/renderer/core/context.hpp>
#include <vkx/renderer/core/profile.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/vkx_exceptions.hpp>
#include <vkx/debug.hpp>

vkx::RendererContext::RendererContext(SDL_Window* window,
                                      Profile const &profile) {
    // Everything that inherits just has one instance of application info as it is the same for everything
    constexpr static const vk::ApplicationInfo applicationInfo{
            "Voxel Game",
            VK_MAKE_VERSION(0, 0, 0),
            "Voxel Engine",
            VK_MAKE_VERSION(0, 0, 0),
            VK_API_VERSION_1_0
    };

    std::uint32_t count = 0;

    // Query the amount of extensions
    if (SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr) != SDL_TRUE) {
        throw vkx::SDLError();
    }

    // Allocate vector
    std::vector<char const *> extensions(count);

    // Query extensions
    if (SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()) != SDL_TRUE) {
        throw vkx::SDLError();
    }

    // If built in debug add debug util extension
#ifdef DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    vk::InstanceCreateInfo instanceCreateInfo{
            {},
            &applicationInfo,
            profile.layers,
            extensions
    };

    instance = createInstance(instanceCreateInfo);
}

std::unordered_map<std::uint32_t, vk::PhysicalDevice>
vkx::RendererContext::getPhysicalDevices(vk::UniqueSurfaceKHR const &surface,
                                         vkx::Profile const &profile) const {
    auto physicalDevices = instance->enumeratePhysicalDevices();
    std::unordered_map<std::uint32_t, vk::PhysicalDevice> ratedPhysicalDevices;
    std::ranges::transform(physicalDevices, std::inserter(ratedPhysicalDevices, ratedPhysicalDevices.begin()),
                           [&surface, &profile](const auto &physicalDevice) {
                               return std::make_pair(ratePhysicalDevice(physicalDevice, surface, profile),
                                                     physicalDevice);
                           });
    return ratedPhysicalDevices;
}

vk::PhysicalDevice
vkx::RendererContext::getBestPhysicalDevice(vk::UniqueSurfaceKHR const &surface,
                                            Profile const &profile) const {
    auto physicalDevices = getPhysicalDevices(surface, profile);

    auto location = std::ranges::max_element(physicalDevices,
                                             [](auto const &lhs, auto const &rhs) { return lhs.first < rhs.first; });
    if (location == physicalDevices.end()) {
        throw vkx::VulkanError("Failed to find physical device.");
    }
    return (*location).second;
}

vk::UniqueSurfaceKHR vkx::RendererContext::createSurface(std::shared_ptr<SDLWindow> const &window) const {
    VkSurfaceKHR surface = nullptr;
    if (SDL_Vulkan_CreateSurface(window->cWindow.get(),
                                 *instance,
                                 &surface) != SDL_TRUE) {
        throw vkx::VulkanError("Failure to create VkSurfaceKHR via the SDL2 API.");
    }
    return vk::UniqueSurfaceKHR(surface, *instance);
}

vk::UniqueSurfaceKHR vkx::RendererContext::createSurface(SDL_Window* window) const {
    VkSurfaceKHR surface = nullptr;
    if (SDL_Vulkan_CreateSurface(window,
                                 *instance,
                                 &surface) != SDL_TRUE) {
        throw vkx::VulkanError("Failure to create VkSurfaceKHR via the SDL2 API.");
    }
    return vk::UniqueSurfaceKHR(surface, *instance);
}

vk::UniqueInstance const &vkx::RendererContext::getInstance() const noexcept {
    return instance;
}

vk::UniqueInstance
vkx::RendererContext::createInstance(vk::InstanceCreateInfo const &instanceCreateInfo) {
#ifdef DEBUG
    auto messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    auto messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
            {},
            messageSeverity,
            messageType,
            vkDebugCallback,
            nullptr
    };

    vk::StructureChain structureChain{
            instanceCreateInfo,
            debugUtilsMessengerCreateInfo};
    return vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
    return vk::createInstanceUnique(instanceCreateInfo);
#endif
}

std::uint32_t
vkx::RendererContext::ratePhysicalDevice(vk::PhysicalDevice const &physicalDevice,
                                         vk::UniqueSurfaceKHR const &surface,
                                         Profile const &profile) {
    std::uint32_t rating = 0;
    if (profile.validateExtensions(physicalDevice)) {
        rating++;
    }

    if (QueueConfig indices{physicalDevice, surface}; indices.isComplete()) {
        rating++;
    }

    if (SwapchainInfo info{physicalDevice, surface}; info.isComplete()) {
        rating++;
    }

    if (physicalDevice.getFeatures().samplerAnisotropy) {
        rating++;
    }

    return rating;
}