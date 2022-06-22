//
// Created by december on 6/21/22.
//

#include <application.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <debug.hpp>
#include <iostream>

vkx::App::App(const vkx::AppConfig &config) {
    auto initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO;
    auto sdlInitCode = SDL_Init(initFlags);

    if (sdlInitCode < 0) {
        throw std::system_error(std::error_code(sdlInitCode, std::generic_category()),SDL_GetError());
    }

    sdlInitCode = SDL_Vulkan_LoadLibrary(nullptr);

    if (sdlInitCode != 0) {
        throw std::system_error(std::error_code(sdlInitCode, std::generic_category()),SDL_GetError());
    }

    window = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.windowWidth, config.windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (window == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.pApplicationName = "Hello World";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "vkx";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    unsigned int sdlExtensionsCount;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, nullptr)) {
        throw std::runtime_error(SDL_GetError());
    }

    std::vector<const char*> sdlExtensions(sdlExtensionsCount);

    if (!SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, sdlExtensions.data())) {
        throw std::runtime_error(SDL_GetError());
    }

    std::vector<const char*> layers = {
#ifdef DEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
    };

#ifdef DEBUG
    sdlExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = {};
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
    instanceCreateInfo.ppEnabledLayerNames = layers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(sdlExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = sdlExtensions.data();

#ifdef DEBUG
    auto debugMessageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    auto debugMessageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
    debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCreateInfo.pNext = nullptr;
    debugUtilsMessengerCreateInfo.flags = {};
    debugUtilsMessengerCreateInfo.messageSeverity = debugMessageSeverity;
    debugUtilsMessengerCreateInfo.messageType = debugMessageType;
    debugUtilsMessengerCreateInfo.pfnUserCallback = vkDebugCallback;
    debugUtilsMessengerCreateInfo.pUserData = this;

    // Create debug utils messenger upon instance creation
    instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
#endif

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failure to create Vulkan instance.");
    }

    if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
        throw std::runtime_error("Failure to create SDL surface.");
    }

    std::uint32_t physicalDeviceCount;
    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failure to count physical devices.");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failure to enumerate physical devices.");
    }

    std::uint32_t bestRating = 0;
    VkPhysicalDevice bestPhysicalDevice = nullptr;
    for (const auto pDevice : physicalDevices) {
        auto newRating = rate(pDevice);
        if (newRating > bestRating) {
            bestRating = newRating;
            bestPhysicalDevice = pDevice;
        }
    }

    if (bestPhysicalDevice == nullptr) {
        throw std::runtime_error("Failure to find suitable device.");
    }
}

vkx::App::~App() {
    // Vulkan clean up
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    // SDL clean up
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

void vkx::App::run() {
//    SDL_ShowWindow(window);
//    bool running = true;
//    SDL_Event event{};
//    while (running) {
//        while (SDL_PollEvent(&event) != 0) {
//            if (event.type == SDL_QUIT) {
//                running = false;
//                break;
//            }
//        }
//    }
}

std::uint32_t vkx::App::rate(VkPhysicalDevice physicalDevice) {
    std::uint32_t rating = 0;

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    return rating;
}
