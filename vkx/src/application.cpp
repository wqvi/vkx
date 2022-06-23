//
// Created by december on 6/21/22.
//

#include <application.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <debug.hpp>
#include <iostream>

VkFormat vkx::findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

VkFormat vkx::findDepthFormat(VkPhysicalDevice physicalDevice) {
    return vkx::findSupportedFormat(physicalDevice,
                               {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

std::uint32_t vkx::findMemoryType(VkPhysicalDevice physicalDevice, std::uint32_t typeFilter, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &props);

    for (std::uint32_t i = 0; i < props.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

    throw std::invalid_argument("Failure to find suitable memory type based on provided parameters.");
}

VkDeviceMemory vkx::allocateMemory(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags flags) {

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, flags);

    VkDeviceMemory memory;
    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("Failure to allocate GPU memory.");
    }

    return memory;
}

VkImage vkx::createImage(VkDevice logicalDevice, std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) {
    VkExtent3D extent{};
    extent.width = width;
    extent.height = height;
    extent.depth = 1;

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = {};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = extent;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    if (vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failure to create image.");
    }

    return image;
}


VkImageView vkx::createImageView(VkDevice logicalDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = aspectFlags;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.flags = {};
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components = {};
    imageViewCreateInfo.subresourceRange = subresourceRange;

    VkImageView imageView;
    if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failure to create image view.");
    }
    return imageView;
}

vkx::SwapChain::SwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, std::uint32_t windowWidth, std::uint32_t windowHeight, const SwapChain &oldSwapChain)
    : logicalDevice(logicalDevice) {
    vkx::DeviceQueueConfig queueConfig = {physicalDevice, surface};
    vkx::SwapchainSurfaceConfig swapchainSurfaceConfig = {physicalDevice, surface};

    auto surfaceFormat = swapchainSurfaceConfig.chooseSurfaceFormat();
    auto presentMode = swapchainSurfaceConfig.choosePresentMode();
    auto actualExtent = swapchainSurfaceConfig.chooseExtent(windowWidth, windowHeight);
    auto imageCount = swapchainSurfaceConfig.getImageCount();
    auto imageSharingMode = queueConfig.getImageSharingMode();
    auto indices = queueConfig.getContiguousIndices();

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.pNext = nullptr;
    swapChainCreateInfo.flags = {};
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = actualExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.imageSharingMode = imageSharingMode;
    swapChainCreateInfo.queueFamilyIndexCount = static_cast<std::uint32_t>(indices.size());
    swapChainCreateInfo.pQueueFamilyIndices = indices.data();
    swapChainCreateInfo.preTransform = swapchainSurfaceConfig.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = nullptr;

    if (oldSwapChain.swapchain != nullptr) {
        swapChainCreateInfo.oldSwapchain = oldSwapChain.swapchain;
    }

    if (vkCreateSwapchainKHR(logicalDevice, &swapChainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failure to create swapchain");
    }

    std::uint32_t swapchainImageCount;
    if (vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failure to get swapchain image count.");
    }

    images.resize(swapchainImageCount);

    if (vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, images.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failure to enumerate swapchain images.");
    }

    imageFormat = surfaceFormat.format;
    extent = actualExtent;

    for (VkImage image : images) {
        auto imageView = vkx::createImageView(logicalDevice, image, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        imageViews.push_back(imageView);
    }

    auto depthFormat = vkx::findDepthFormat(physicalDevice);
    depthImage = createImage(logicalDevice, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(logicalDevice, depthImage, &requirements);
    depthImageMemory = allocateMemory(physicalDevice, logicalDevice, requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkBindImageMemory(logicalDevice, depthImage, depthImageMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Failure to bind image memory");
    }

    depthImageView = createImageView(logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

vkx::SwapChain::~SwapChain() {
    if (swapchain != nullptr) {
        vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
    }
}

void vkx::SwapChain::createFramebuffers(VkRenderPass renderPass) {
    framebuffers.resize(imageViews.size());

    for (std::size_t i = 0; i < imageViews.size(); i++) {
        std::vector<VkImageView> framebufferAttachments{
            imageViews[i],
            depthImageView};

        VkFramebufferCreateInfo framebufferCreateInfo{};

    }
}

vkx::App::App(const vkx::AppConfig &config) {
    auto sdlInitCode = SDL_Init(SDL_INIT_EVERYTHING);

    if (sdlInitCode < 0) {
        throw std::system_error(std::error_code(sdlInitCode, std::generic_category()), SDL_GetError());
    }

    auto windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
    window = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.windowWidth,
                              config.windowHeight, windowFlags);

    if (window == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }

    std::cout << SDL_GetCurrentVideoDriver() << '\n';

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

    std::vector<const char *> sdlExtensions(sdlExtensionsCount);

    if (!SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionsCount, sdlExtensions.data())) {
        throw std::runtime_error(SDL_GetError());
    }

    std::vector<const char *> layers = {
#ifdef DEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
    };

    std::uint32_t instanceLayersCount;
    if (vkEnumerateInstanceLayerProperties(&instanceLayersCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failure to get instance layer count.");
    }

    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayersCount);

    if (vkEnumerateInstanceLayerProperties(&instanceLayersCount, instanceLayerProperties.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failure to enumerate instance layers.");
    }

    std::vector<const char *> arr;
    std::transform(instanceLayerProperties.begin(), instanceLayerProperties.end(), std::back_inserter(arr),
                   [](const auto &properties) {
                       return properties.layerName;
                   });

    if (!isSubset(arr, layers)) {
        throw std::runtime_error("Failure to find layer support.");
    }

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
        throw std::runtime_error("Failure to get physical device count.");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failure to enumerate physical devices.");
    }

    std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef DEBUG
    std::size_t dbgDeviceCount = 0;
#endif

    std::uint32_t bestRating = 0;
    VkPhysicalDevice bestPhysicalDevice = nullptr;
    for (const auto pDevice: physicalDevices) {
#ifdef DEBUG
        std::cout << "Looking for devices! [" << dbgDeviceCount + 1 << '/' << physicalDeviceCount << "]\n";
#endif
        auto newRating = rate(pDevice, surface, deviceExtensions);
        if (newRating > bestRating) {
            bestRating = newRating;
            bestPhysicalDevice = pDevice;
        }

#ifdef DEBUG
        dbgDeviceCount++;
#endif
    }

    if (bestPhysicalDevice == nullptr) {
        throw std::runtime_error("Failure to find suitable device.");
    }

    physicalDevice = bestPhysicalDevice;

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    std::cout << "Found device ";
    std::cout << physicalDeviceProperties.deviceName << '\n';

    vkx::DeviceQueueConfig deviceQueueConfig = {physicalDevice, surface};
    vkx::SwapchainSurfaceConfig swapchainSurfaceConfig = {physicalDevice, surface};

    float queuePriority = 1.0f;
    auto queueCreateInfos = deviceQueueConfig.createQueueInfos(&queuePriority);

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = true;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = {};
    deviceCreateInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
    deviceCreateInfo.ppEnabledLayerNames = layers.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failure to create logical device.");
    }
}

vkx::App::~App() {
    // Vulkan clean up
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    // SDL clean up
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void vkx::App::run() {
    SDL_ShowWindow(window);
    bool running = true;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        // Redraw
    }
}

std::uint32_t
vkx::App::rate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char *> &extensions) {
    std::uint32_t physicalDeviceExtensionsCount;
    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &physicalDeviceExtensionsCount, nullptr) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failure to get physical device extension count.");
    }

    std::vector<VkExtensionProperties> physicalDeviceExtensions(physicalDeviceExtensionsCount);

    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &physicalDeviceExtensionsCount,
                                             physicalDeviceExtensions.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failure to enumerate physical devices.");
    }

    std::vector<const char *> arr;
    std::transform(physicalDeviceExtensions.begin(), physicalDeviceExtensions.end(), std::back_inserter(arr),
                   [](const auto &properties) {
                       return properties.extensionName;
                   });

    if (!isSubset(arr, extensions)) {
        return 0;
    }

    std::uint32_t rating = 0;

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    if (physicalDeviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM) {
        rating += physicalDeviceProperties.deviceType;
    }

    vkx::DeviceQueueConfig queueConfig = {physicalDevice, surface};
    if (!queueConfig.isComplete()) {
        return 0;
    }

    vkx::SwapchainSurfaceConfig surfaceConfig = {physicalDevice, surface};
    if (!surfaceConfig.isComplete()) {
        return 0;
    }

    return rating;
}

bool vkx::App::isSubset(const std::vector<const char *> &arr, const std::vector<const char *> &subset) {
    std::size_t i = 0;
    for (const char *subsetStr: subset) {
        for (i = 0; i < arr.size(); i++) {
            if (std::strcmp(arr[i], subsetStr) == 0) {
                break;
            }
        }

        if (i == arr.size()) {
            return false;
        }
    }
    return true;
}

vkx::DeviceQueueConfig::DeviceQueueConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    std::uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    if (queueFamilyCount < 1) {
        throw std::runtime_error("Unable to create device queue configuration from zero queue families.");
    }

    VkBool32 support = VK_FALSE;
    for (std::uint32_t i = 0; i < queueFamilyCount; i++) {
        auto currentQueueFamily = queueFamilyProperties[static_cast<std::size_t>(i)];

        if (currentQueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            computeQueueIndex = i;
        }

        if (currentQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &support);
        if (support) {
            presentQueueIndex = i;
        }

        if (isComplete()) {
            break;
        }
    }
}

std::vector<std::uint32_t> vkx::DeviceQueueConfig::getContiguousIndices() const {
    std::set<std::uint32_t> indices = {*computeQueueIndex, *graphicsQueueIndex, *presentQueueIndex};
    std::vector<std::uint32_t> uniqueIndices(indices.begin(), indices.end());
    return uniqueIndices;
}

std::vector<VkDeviceQueueCreateInfo> vkx::DeviceQueueConfig::createQueueInfos(const float *queuePriority) const {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    auto indices = getContiguousIndices();
    for (std::uint32_t index : indices) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = {};
        queueCreateInfo.queueFamilyIndex = index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    return queueCreateInfos;
}

[[nodiscard]] bool vkx::DeviceQueueConfig::isUniversal() const {
    return *computeQueueIndex == *graphicsQueueIndex && *graphicsQueueIndex == *presentQueueIndex;
}

[[nodiscard]] VkSharingMode vkx::DeviceQueueConfig::getImageSharingMode() const
{
    if (isUniversal()) {
        return VK_SHARING_MODE_EXCLUSIVE;
    }
    return VK_SHARING_MODE_CONCURRENT;
}

bool vkx::DeviceQueueConfig::isComplete() const {
    return computeQueueIndex.has_value() && graphicsQueueIndex.has_value() && presentQueueIndex.has_value();
}

vkx::SwapchainSurfaceConfig::SwapchainSurfaceConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    std::uint32_t formatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);

    formats.resize(formatsCount);

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, formats.data());

    std::uint32_t presentModesCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);

    presentModes.resize(presentModesCount);

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, presentModes.data());
}

VkSurfaceFormatKHR vkx::SwapchainSurfaceConfig::chooseSurfaceFormat() const {
    for (const VkSurfaceFormatKHR &surfaceFormat: formats) {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surfaceFormat;
        }
    }

    return formats.at(0);
}

VkPresentModeKHR vkx::SwapchainSurfaceConfig::choosePresentMode() const {
    for (VkPresentModeKHR presentMode: presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vkx::SwapchainSurfaceConfig::chooseExtent(std::uint32_t width, std::uint32_t height) const {
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    VkExtent2D extent{};
    extent.width = width;
    extent.height = height;

    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}

std::uint32_t vkx::SwapchainSurfaceConfig::getImageCount() const {
    std::uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        return capabilities.maxImageCount;
    }
    return imageCount;
}

bool vkx::SwapchainSurfaceConfig::isComplete() const {
    return !formats.empty() && !presentModes.empty();
}