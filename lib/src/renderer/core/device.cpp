#include <renderer/core/device.hpp>

#include <renderer/core/swapchain_info.hpp>
#include <renderer/core/commands.hpp>
#include <renderer/core/swapchain_info.hpp>

namespace vkx
{
    Device::Device(vk::UniqueInstance const &instance, vk::UniqueSurfaceKHR const &surface, Profile const &profile)
    {
        for (auto physicalDevices = instance->enumeratePhysicalDevices(); auto const &current : physicalDevices)
        {
            if (validatePhysicalDevice(current, surface, profile))
            {
                physicalDevice = current;
                break;
            }
        }

        properties = physicalDevice.getProperties();

        QueueConfig queueConfig{physicalDevice, surface};
        if (!queueConfig.isComplete())
        {
            throw std::runtime_error("Something went terribly wrong. Failure to find queue configuration.");
        }

        float queuePriority = 1.0f;
        auto queueCreateInfos = queueConfig.createQueueInfos(queuePriority);

        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = true;

        vk::DeviceCreateInfo deviceCreateInfo{
            {},                 // flags
            queueCreateInfos,   // queueCreateInfos
            profile.layers,     // pEnabledLayerNames
            profile.extensions, // pEnabledExtensionNames
            &deviceFeatures     // pEnabledFeatures
        };

        device = physicalDevice.createDeviceUnique(deviceCreateInfo);

        vk::CommandPoolCreateInfo commandPoolInfo{
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer, // flags
            queueConfig.graphicsIndex.value()                   // queueFamilyIndex
        };

        commandPool = device->createCommandPoolUnique(commandPoolInfo);

        queues = Queues(*this, queueConfig);
    }

    Device::Device(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface, Profile const &profile)
        : physicalDevice(physicalDevice)
    {
        properties = physicalDevice.getProperties();

        QueueConfig queueConfig{physicalDevice, surface};
        if (!queueConfig.isComplete())
        {
            throw std::runtime_error("Something went terribly wrong. Failure to find queue configuration.");
        }

        float queuePriority = 1.0f;
        auto queueCreateInfos = queueConfig.createQueueInfos(queuePriority);

        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = true;

        vk::DeviceCreateInfo deviceCreateInfo{
            {},                 // flags
            queueCreateInfos,   // queueCreateInfos
            profile.layers,     // pEnabledLayerNames
            profile.extensions, // pEnabledExtensionNames
            &deviceFeatures     // pEnabledFeatures
        };

        device = physicalDevice.createDeviceUnique(deviceCreateInfo);

        vk::CommandPoolCreateInfo commandPoolInfo{
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer, // flags
            queueConfig.graphicsIndex.value()                   // queueFamilyIndex
        };

        commandPool = device->createCommandPoolUnique(commandPoolInfo);

        queues = Queues(*this, queueConfig);
    }

    Device::operator vk::PhysicalDevice const &() const
    {
        return physicalDevice;
    }

    Device::operator vk::Device const &() const
    {
        return *device;
    }

    Device::operator vk::CommandPool const &() const
    {
        return *commandPool;
    }

    Device::operator vk::UniqueCommandPool const &() const
    {
        return commandPool;
    }

    Device::operator vk::UniqueDevice const &() const
    {
        return device;
    }

    vk::Device const &Device::operator*() const
    {
        return *device;
    }

    vk::Device const *Device::operator->() const
    {
        return &*device;
    }

    std::vector<DrawCommand> Device::createDrawCommands(std::uint32_t size) const
    {
        vk::CommandBufferAllocateInfo allocInfo{
            *commandPool,                     // commandPool
            vk::CommandBufferLevel::ePrimary, // level
            size                              // commandBufferCount
        };

        auto commandBuffers = device->allocateCommandBuffers(allocInfo);

        std::vector<DrawCommand> drawCommands;
        std::transform(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(drawCommands), [&device = this->device](auto const &commandBuffer) -> DrawCommand
                       { return {device, commandBuffer}; });
        return drawCommands;
    }

    std::uint32_t Device::findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags const &flags) const
    {
        auto memProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type.");
    }

    vk::Format Device::findSupportedFormat(std::vector<vk::Format> const &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags const &features) const
    {
        for (vk::Format format : candidates)
        {
            auto formatProps = physicalDevice.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (formatProps.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == vk::ImageTiling::eOptimal && (formatProps.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        return vk::Format::eUndefined;
    }

    vk::Format Device::findDepthFormat() const
    {
        return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                   vk::ImageTiling::eOptimal,
                                   vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }

    vk::UniqueImage Device::createImageUnique(std::uint32_t width, std::uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags const &usage) const
    {
        vk::Extent3D extent{
            width,  // width
            height, // height
            1       // depth
        };

        vk::ImageCreateInfo imageInfo{
            {},                          // flags
            vk::ImageType::e2D,          // imageType
            format,                      // format
            extent,                      // extent
            1,                           // mipLevels
            1,                           // arrayLayers
            vk::SampleCountFlagBits::e1, // samples
            tiling,                      // tiling
            usage,                       // usage
            vk::SharingMode::eExclusive, // sharing
            {},                          // queueFamilyIndices
            vk::ImageLayout::eUndefined  // initialLayout
        };

        return device->createImageUnique(imageInfo);
    }

    vk::UniqueBuffer Device::createBufferUnique(vk::DeviceSize size, vk::BufferUsageFlags const &usage) const
    {
        vk::BufferCreateInfo bufferInfo{
            {},                         // flags
            size,                       // size,
            usage,                      // usage
            vk::SharingMode::eExclusive // sharingMode
        };

        return device->createBufferUnique(bufferInfo);
    }

    vk::UniqueDeviceMemory Device::allocateMemoryUnique(vk::UniqueBuffer const &buffer, vk::MemoryPropertyFlags const &flags) const
    {
        auto memReqs = device->getBufferMemoryRequirements(*buffer);

        vk::MemoryAllocateInfo allocInfo{
            memReqs.size,                                 // allocationSize
            findMemoryType(memReqs.memoryTypeBits, flags) // memoryTypeIndex
        };

        auto memory = device->allocateMemoryUnique(allocInfo);

        device->bindBufferMemory(*buffer, *memory, 0);

        return memory;
    }

    vk::UniqueDeviceMemory Device::allocateMemoryUnique(vk::UniqueImage const &image, vk::MemoryPropertyFlags const &flags) const
    {
        auto memReqs = device->getImageMemoryRequirements(*image);

        vk::MemoryAllocateInfo allocInfo{
            memReqs.size,                                 // allocationSize
            findMemoryType(memReqs.memoryTypeBits, flags) // memoryTypeIndex
        };

        auto memory = device->allocateMemoryUnique(allocInfo);

        device->bindImageMemory(*image, *memory, 0);

        return memory;
    }

    vk::UniqueImageView Device::createImageViewUnique(vk::Image const &image, vk::Format format, vk::ImageAspectFlags const &aspectFlags) const
    {
        vk::ImageSubresourceRange subresourceRange{
            aspectFlags, // aspectMask
            0,           // baseMipLevel
            1,           // levelCount
            0,           // baseArrayLayer
            1            // layerCount
        };

        vk::ImageViewCreateInfo imageViewInfo{
            {},                     // flags
            image,                  // image
            vk::ImageViewType::e2D, // viewType
            format,                 // format
            {},                     // components
            subresourceRange        // subresourceRange
        };

        return device->createImageViewUnique(imageViewInfo);
    }

    void Device::copyBuffer(
        vk::Buffer const &srcBuffer,
        vk::Buffer const &dstBuffer,
        vk::DeviceSize const &size) const
    {
        SingleTimeCommand singleTimeCommand{*this, queues.graphics};

        vk::BufferCopy copyRegion{};
        copyRegion.size = size;
        singleTimeCommand->copyBuffer(srcBuffer, dstBuffer, copyRegion);
    }

    void Device::copyBufferToImage(
        vk::Buffer const &buffer,
        vk::Image const &image,
        std::uint32_t width,
        std::uint32_t height) const
    {
        SingleTimeCommand singleTimeCommand{*this, queues.graphics};

        vk::ImageSubresourceLayers subresourceLayer{
            vk::ImageAspectFlagBits::eColor, // aspectMask
            0,                               // mipLevel
            0,                               // baseArrayLayer
            1                                // layerCount
        };

        vk::Offset3D imageOffset{
            0, // x
            0, // y
            0  // z
        };
        vk::Extent3D imageExtent{
            width,  // width
            height, // height
            1       // depth
        };

        vk::BufferImageCopy region{
            0,                // bufferOffset
            0,                // bufferRowLength
            0,                // bufferImageHeight
            subresourceLayer, // imageSubresource
            imageOffset,      // imageOffset
            imageExtent       // imageExtent
        };

        singleTimeCommand->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    }

    void Device::transitionImageLayout(
        vk::Image const &image,
        vk::ImageLayout const &oldLayout,
        vk::ImageLayout const &newLayout) const
    {
        SingleTimeCommand singleTimeCommand{*this, queues.graphics};

        vk::ImageSubresourceRange subresourceRange{
            vk::ImageAspectFlagBits::eColor, // aspectMask
            0,                               // baseMipLevel
            1,                               // levelCount
            0,                               // baseArrayLayer
            1                                // layerCount
        };

        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            srcAccessMask = {};
            dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else
        {
            throw std::invalid_argument("Unsupported layout transition.");
        }

        vk::ImageMemoryBarrier barrier{
            srcAccessMask,           // srcAccessMask
            dstAccessMask,           // dstAccessMask
            oldLayout,               // oldLayout
            newLayout,               // newLayout
            VK_QUEUE_FAMILY_IGNORED, // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED, // dstQueueFamilyIndex
            image,                   // image
            subresourceRange         // subresourceRange
        };

        singleTimeCommand->pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
    }

    void Device::submit(std::vector<vk::CommandBuffer> const &commandBuffers, vk::Semaphore const &waitSemaphore, vk::Semaphore const &signalSemaphore, vk::Fence const &flightFence) const
    {
        std::array<vk::PipelineStageFlags, 1> waitStage{
            vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo submitInfo{
            1,                                                 // waitSemaphoreCount
            &waitSemaphore,                                    // pWaitSemaphores
            waitStage.data(),                                  // pWaitDstStageMask
            static_cast<std::uint32_t>(commandBuffers.size()), // commandBufferCount
            commandBuffers.data(),                             // pCommandBuffers
            1,                                                 // signalSemaphoreCount
            &signalSemaphore                                   // pSignalSemaphores
        };

        queues.graphics.submit(submitInfo, flightFence);
    }

    vk::Result Device::present(vk::SwapchainKHR const &swapchain, std::uint32_t imageIndex, vk::Semaphore const &signalSemaphores) const
    {
        vk::PresentInfoKHR presentInfo{
            1,                 // waitSemaphoreCount
            &signalSemaphores, // pWaitSemaphores
            1,                 // swapchainCount
            &swapchain,        // pSwapchains
            &imageIndex        // pImageIndices
        };

        return queues.present.presentKHR(&presentInfo);
    }

    [[nodiscard]] vk::UniqueImageView Device::createTextureImageViewUnique(vk::Image const &image) const
    {
        return createImageViewUnique(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    }

    [[nodiscard]] vk::UniqueSampler Device::createTextureSamplerUnique() const
    {
        vk::SamplerCreateInfo samplerInfo{
            {},                                     // flags
            vk::Filter::eLinear,                    // magFilter
            vk::Filter::eLinear,                    // minFilter
            vk::SamplerMipmapMode::eLinear,         // mipmapMode
            vk::SamplerAddressMode::eRepeat,        // addressModeU
            vk::SamplerAddressMode::eRepeat,        // addressModeV
            vk::SamplerAddressMode::eRepeat,        // addressModeW
            {},                                     // mipLodBias
            true,                                   // anisotropyEnable
            properties.limits.maxSamplerAnisotropy, // maxAnisotropy
            false,                                  // compareEnable
            vk::CompareOp::eAlways,                 // compareOp
            {},                                     // minLod
            {},                                     // maxLod
            vk::BorderColor::eIntOpaqueBlack,       // borderColor
            false                                   // unnormalizedCoordinates
        };

        return device->createSamplerUnique(samplerInfo);
    }

    bool Device::validatePhysicalDevice(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface, Profile const &profile)
    {
        if (!profile.validateExts(physicalDevice))
        {
            return false;
        }

        if (QueueConfig indices{physicalDevice, surface}; !indices.isComplete())
        {
            return false;
        }

        if (SwapchainInfo info{physicalDevice, surface}; !info.isComplete())
        {
            return false;
        }

        return physicalDevice.getFeatures().samplerAnisotropy;
    }
}