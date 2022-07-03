#include <renderer/core/swapchain.hpp>

#include <renderer/core/swapchain_info.hpp>
#include <renderer/core/queue_config.hpp>

namespace vkx
{
	Swapchain::Swapchain() = default;

	Swapchain::Swapchain(
			Device const &device,
			vk::UniqueSurfaceKHR const &surface,
			SDL_Window *window,
			Swapchain const &oldSwapchain)
	{
		SwapchainInfo info{static_cast<vk::PhysicalDevice>(device), surface};
		QueueConfig config{static_cast<vk::PhysicalDevice>(device), surface};

        int width;
        int height;
        SDL_Vulkan_GetDrawableSize(window, &width, &height);

		auto surfaceFormat = info.chooseSurfaceFormat();
		auto presentMode = info.choosePresentMode();
		auto actualExtent = info.chooseExtent(width, height);

		auto imageCount = info.getImageCount();
		auto imageSharingMode = config.getImageSharingMode();

		vk::SwapchainCreateInfoKHR swapchainCreateInfo{
				{},																				// flags
				*surface,																	// surface
				imageCount,																// minImageCount
				surfaceFormat.format,											// imageFormat
				surfaceFormat.colorSpace,									// imageColorSpace
				actualExtent,															// imageExtent,
				1,																				// imageArrayLayers
				vk::ImageUsageFlagBits::eColorAttachment, // imageUsage
				imageSharingMode,													// imageSharingMode
				config.indices,														// queueFamilyIndices
				info.capabilities.currentTransform,				// preTransform
				vk::CompositeAlphaFlagBitsKHR::eOpaque,		// compositeAlpha
				presentMode,															// presentMode,
				true																			// clipped
		};

		// According to Vulkan spec providing the old swapchain can prove beneficial.
		// It aids in resource reusing. While this is only really useful during resizing the window it's good practice.
		if (static_cast<bool>(oldSwapchain.swapchain))
		{
			swapchainCreateInfo.oldSwapchain = *oldSwapchain.swapchain;
		}

		swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);

		images = device->getSwapchainImagesKHR(*swapchain);

		imageFormat = surfaceFormat.format;
		extent = actualExtent;

		for (auto const &image : images)
		{
			imageViews.push_back(device.createImageViewUnique(image, imageFormat, vk::ImageAspectFlagBits::eColor));
		}

		auto depthFormat = device.findDepthFormat();
		depthImage = device.createImageUnique(extent.width, extent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
		depthImageMemory = device.allocateMemoryUnique(depthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
		depthImageView = device.createImageViewUnique(*depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
	}

    Swapchain::Swapchain(const Device &device,
              const vk::UniqueSurfaceKHR &surface,
              const SDLWindow &window,
              const Swapchain &oldSwapchain)
    {
        SwapchainInfo info{static_cast<vk::PhysicalDevice>(device), surface};
        QueueConfig config{static_cast<vk::PhysicalDevice>(device), surface};

        const auto [width, height] = window.getSize();

        auto surfaceFormat = info.chooseSurfaceFormat();
        auto presentMode = info.choosePresentMode();
        auto actualExtent = info.chooseExtent(width, height);

        auto imageCount = info.getImageCount();
        auto imageSharingMode = config.getImageSharingMode();

        vk::SwapchainCreateInfoKHR swapchainCreateInfo{
                {},																				// flags
                *surface,																	// surface
                imageCount,																// minImageCount
                surfaceFormat.format,											// imageFormat
                surfaceFormat.colorSpace,									// imageColorSpace
                actualExtent,															// imageExtent,
                1,																				// imageArrayLayers
                vk::ImageUsageFlagBits::eColorAttachment, // imageUsage
                imageSharingMode,													// imageSharingMode
                config.indices,														// queueFamilyIndices
                info.capabilities.currentTransform,				// preTransform
                vk::CompositeAlphaFlagBitsKHR::eOpaque,		// compositeAlpha
                presentMode,															// presentMode,
                true																			// clipped
        };

        // According to Vulkan spec providing the old swapchain can prove beneficial.
        // It aids in resource reusing. While this is only really useful during resizing the window it's good practice.
        if (static_cast<bool>(oldSwapchain.swapchain))
        {
            swapchainCreateInfo.oldSwapchain = *oldSwapchain.swapchain;
        }

        swapchain = device->createSwapchainKHRUnique(swapchainCreateInfo);

        images = device->getSwapchainImagesKHR(*swapchain);

        imageFormat = surfaceFormat.format;
        extent = actualExtent;

        for (auto const &image : images)
        {
            imageViews.push_back(device.createImageViewUnique(image, imageFormat, vk::ImageAspectFlagBits::eColor));
        }

        auto depthFormat = device.findDepthFormat();
        depthImage = device.createImageUnique(extent.width, extent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
        depthImageMemory = device.allocateMemoryUnique(depthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
        depthImageView = device.createImageViewUnique(*depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
    }

	Swapchain::operator vk::SwapchainKHR const &() const
	{
		return *swapchain;
	}

	Swapchain::operator vk::UniqueSwapchainKHR const &() const
	{
		return swapchain;
	}

	void Swapchain::createFramebuffers(Device const &device, vk::UniqueRenderPass const &renderPass)
	{
		framebuffers.resize(imageViews.size());

		for (std::size_t i = 0; i < imageViews.size(); i++)
		{
			std::vector<vk::ImageView> framebufferAttachments{
					*imageViews[i],
					*depthImageView};

			vk::FramebufferCreateInfo framebufferInfo{
					{},											// flags
					*renderPass,						// renderPass
					framebufferAttachments, // attachments
					extent.width,						// width
					extent.height,					// height
					1												// layers
			};

			framebuffers[i] = device->createFramebufferUnique(framebufferInfo);
		}
	}

	vk::ResultValue<std::uint32_t> Swapchain::acquireNextImage(Device const &device, vk::UniqueSemaphore const &semaphore) const
	{
		std::uint32_t imageIndex = 0;
		auto const result = device->acquireNextImageKHR(*swapchain, UINT64_MAX, *semaphore, {}, &imageIndex);
		return {result, imageIndex};
	}

}