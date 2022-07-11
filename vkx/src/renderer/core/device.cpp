#include <renderer/core/device.hpp>

#include <renderer/core/swapchain_info.hpp>
#include <renderer/core/commands.hpp>
#include <renderer/core/sync_objects.hpp>
#include <vkx_exceptions.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

void vkx::Device::VmaAllocatorDeleter::operator()(VmaAllocator allocator) const noexcept {
    vmaDestroyAllocator(allocator);
}

vkx::Device::Device(vk::UniqueInstance const &instance,
                    vk::PhysicalDevice const &physicalDevice,
                    vk::UniqueSurfaceKHR const &surface,
                    Profile const &profile)
        : physicalDevice(physicalDevice), properties(physicalDevice.getProperties()) {

    QueueConfig queueConfig{physicalDevice, surface};
    if (!queueConfig.isComplete()) {
        throw vkx::VulkanError("Something went terribly wrong. Failure to find queue configuration.");
    }

    float queuePriority = 1.0f;
    auto queueCreateInfos = queueConfig.createQueueInfos(queuePriority);

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = true;

    vk::DeviceCreateInfo deviceCreateInfo{
            {},
            queueCreateInfos,
            profile.layers,
            profile.extensions,
            &deviceFeatures
    };

    device = physicalDevice.createDeviceUnique(deviceCreateInfo);

    vk::CommandPoolCreateInfo commandPoolInfo{
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            queueConfig.graphicsIndex.value()
    };

    commandPool = device->createCommandPoolUnique(commandPoolInfo);

    queues = Queues(*this, queueConfig);

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT; // Allow multithreading memory
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = *device;
    allocatorCreateInfo.instance = *instance;

    VmaAllocator allocator;
    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
        throw vkx::VulkanError("Failure to create vulkan memory allocator.");
    }

    memoryAllocator = std::unique_ptr<VmaAllocator_T, VmaAllocatorDeleter>(allocator);
}

vkx::Device::operator vk::PhysicalDevice const &() const {
    return physicalDevice;
}

vkx::Device::operator vk::Device const &() const {
    return *device;
}

vkx::Device::operator vk::CommandPool const &() const {
    return *commandPool;
}

vkx::Device::operator vk::UniqueCommandPool const &() const {
    return commandPool;
}

vk::Device const &vkx::Device::operator*() const {
    return *device;
}

vk::Device const *vkx::Device::operator->() const {
    return &*device;
}

std::vector<vkx::DrawCommand> vkx::Device::createDrawCommands(std::uint32_t size) const {
    vk::CommandBufferAllocateInfo allocInfo{
            *commandPool,
            vk::CommandBufferLevel::ePrimary,
            size
    };

    auto commandBuffers = device->allocateCommandBuffers(allocInfo);

    std::vector<DrawCommand> drawCommands;
    std::ranges::transform(commandBuffers, std::back_inserter(drawCommands),
                           [&device = this->device](auto const &commandBuffer) -> DrawCommand {
                               return vkx::DrawCommand{device, commandBuffer};
                           });
    return drawCommands;
}

std::uint32_t vkx::Device::findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags const &flags) const {
    auto memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

    throw vkx::VulkanError("Failure to find suitable physical device memory type.");
}

vk::Format vkx::Device::findSupportedFormat(std::vector<vk::Format> const &candidates, vk::ImageTiling tiling,
                                            vk::FormatFeatureFlags const &features) const {
    for (vk::Format format: candidates) {
        auto formatProps = physicalDevice.getFormatProperties(format);

        bool isLinear = tiling == vk::ImageTiling::eLinear &&
                        (formatProps.linearTilingFeatures & features) == features;
        bool isOptimal = tiling == vk::ImageTiling::eOptimal &&
                         (formatProps.optimalTilingFeatures & features) == features;

        if (isLinear || isOptimal) {
            return format;
        }
    }

    return vk::Format::eUndefined;
}

vk::Format vkx::Device::findDepthFormat() const {
    return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                               vk::ImageTiling::eOptimal,
                               vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::UniqueImage
vkx::Device::createImageUnique(std::uint32_t width, std::uint32_t height, vk::Format format, vk::ImageTiling tiling,
                               vk::ImageUsageFlags const &usage) const {
    vk::Extent3D extent{
            width,
            height,
            1
    };

    vk::ImageCreateInfo imageInfo{
            {},
            vk::ImageType::e2D,
            format,
            extent,
            1,
            1,
            vk::SampleCountFlagBits::e1,
            tiling,
            usage,
            vk::SharingMode::eExclusive,
            {},
            vk::ImageLayout::eUndefined
    };

    return device->createImageUnique(imageInfo);
}

vk::UniqueBuffer vkx::Device::createBufferUnique(vk::DeviceSize size, vk::BufferUsageFlags const &usage) const {
    vk::BufferCreateInfo bufferInfo{
            {},
            size,
            usage,
            vk::SharingMode::eExclusive
    };

    return device->createBufferUnique(bufferInfo);
}

vk::UniqueDeviceMemory
vkx::Device::allocateMemoryUnique(vk::UniqueBuffer const &buffer, vk::MemoryPropertyFlags const &flags) const {
    auto memReqs = device->getBufferMemoryRequirements(*buffer);

    vk::MemoryAllocateInfo allocInfo{
            memReqs.size,
            findMemoryType(memReqs.memoryTypeBits, flags)
    };

    auto memory = device->allocateMemoryUnique(allocInfo);

    device->bindBufferMemory(*buffer, *memory, 0);

    return memory;
}

vk::UniqueDeviceMemory
vkx::Device::allocateMemoryUnique(vk::UniqueImage const &image, vk::MemoryPropertyFlags const &flags) const {
    auto memReqs = device->getImageMemoryRequirements(*image);

    vk::MemoryAllocateInfo allocInfo{
            memReqs.size,
            findMemoryType(memReqs.memoryTypeBits, flags)
    };

    auto memory = device->allocateMemoryUnique(allocInfo);

    device->bindImageMemory(*image, *memory, 0);

    return memory;
}

vk::UniqueImageView vkx::Device::createImageViewUnique(vk::Image const &image, vk::Format format,
                                                       vk::ImageAspectFlags const &aspectFlags) const {
    vk::ImageSubresourceRange subresourceRange{
            aspectFlags,
            0,
            1,
            0,
            1
    };

    vk::ImageViewCreateInfo imageViewInfo{
            {},
            image,
            vk::ImageViewType::e2D,
            format,
            {},
            subresourceRange
    };

    return device->createImageViewUnique(imageViewInfo);
}

void vkx::Device::copyBuffer(
        vk::Buffer const &srcBuffer,
        vk::Buffer const &dstBuffer,
        vk::DeviceSize const &size) const {
    SingleTimeCommand singleTimeCommand{*this, queues.graphics};

    vk::BufferCopy copyRegion{};
    copyRegion.size = size;
    singleTimeCommand->copyBuffer(srcBuffer, dstBuffer, copyRegion);
}

void vkx::Device::copyBufferToImage(
        vk::Buffer const &buffer,
        vk::Image const &image,
        std::uint32_t width,
        std::uint32_t height) const {
    SingleTimeCommand singleTimeCommand{*this, queues.graphics};

    vk::ImageSubresourceLayers subresourceLayer{
            vk::ImageAspectFlagBits::eColor,
            0,
            0,
            1
    };

    vk::Offset3D imageOffset{
            0,
            0,
            0
    };
    vk::Extent3D imageExtent{
            width,
            height,
            1
    };

    vk::BufferImageCopy region{
            0,
            0,
            0,
            subresourceLayer,
            imageOffset,
            imageExtent
    };

    singleTimeCommand->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

void vkx::Device::transitionImageLayout(
        vk::Image const &image,
        vk::ImageLayout const &oldLayout,
        vk::ImageLayout const &newLayout) const {
    SingleTimeCommand singleTimeCommand{*this, queues.graphics};

    vk::ImageSubresourceRange subresourceRange{
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1
    };

    vk::AccessFlags srcAccessMask;
    vk::AccessFlags dstAccessMask;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        srcAccessMask = {};
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("Unsupported layout transition.");
    }

    vk::ImageMemoryBarrier barrier{
            srcAccessMask,
            dstAccessMask,
            oldLayout,
            newLayout,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            image,
            subresourceRange
    };

    singleTimeCommand->pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}

void vkx::Device::submit(std::vector<vk::CommandBuffer> const &commandBuffers, vk::Semaphore const &waitSemaphore,
                         vk::Semaphore const &signalSemaphore, vk::Fence const &flightFence) const {
    std::array<vk::PipelineStageFlags, 1> waitStage{
            vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submitInfo{
            1,
            &waitSemaphore,
            waitStage.data(),
            static_cast<std::uint32_t>(commandBuffers.size()),
            commandBuffers.data(),
            1,
            &signalSemaphore
    };

    queues.graphics.submit(submitInfo, flightFence);
}

vk::Result vkx::Device::present(vk::SwapchainKHR const &swapchain, std::uint32_t imageIndex,
                                vk::Semaphore const &signalSemaphores) const {
    vk::PresentInfoKHR presentInfo{
            1,
            &signalSemaphores,
            1,
            &swapchain,
            &imageIndex
    };

    return queues.present.presentKHR(&presentInfo);
}

[[nodiscard]] vk::UniqueImageView vkx::Device::createTextureImageViewUnique(vk::Image const &image) const {
    return createImageViewUnique(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

[[nodiscard]] vk::UniqueSampler vkx::Device::createTextureSamplerUnique() const {
    vk::SamplerCreateInfo samplerInfo{
            {},
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            {},
            true,
            properties.limits.maxSamplerAnisotropy,
            false,
            vk::CompareOp::eAlways,
            {},
            {},
            vk::BorderColor::eIntOpaqueBlack,
            false
    };

    return device->createSamplerUnique(samplerInfo);
}

void vkx::Device::submit(
        std::vector<DrawCommand> const &drawCommands,
        SyncObjects const &syncObjects) const {
    std::array<vk::PipelineStageFlags, 1> waitStage{
            vk::PipelineStageFlagBits::eColorAttachmentOutput};

    std::vector<vk::CommandBuffer> commands;
    std::ranges::transform(drawCommands, std::back_inserter(commands), [](auto const &drawCommand) {
        return static_cast<vk::CommandBuffer>(drawCommand);
    });

    vk::SubmitInfo submitInfo{
            1,
            &*syncObjects.imageAvailableSemaphore,
            waitStage.data(),
            static_cast<std::uint32_t>(commands.size()),
            commands.data(),
            1,
            &*syncObjects.renderFinishedSemaphore
    };

    queues.graphics.submit(submitInfo, *syncObjects.inFlightFence);
}
