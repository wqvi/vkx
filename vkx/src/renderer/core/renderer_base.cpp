#include <renderer/core/renderer_base.hpp>

#include <renderer/core/swapchain_info.hpp>
#include <renderer/core/queue_config.hpp>
#include <renderer/core/commands.hpp>
#include <renderer/uniform_buffer.hpp>
#include <renderer/model.hpp>

vkx::SyncObjects::SyncObjects(const Device &device)
        : imageAvailableSemaphore(device->createSemaphoreUnique({})),
          renderFinishedSemaphore(device->createSemaphoreUnique({})),
          inFlightFence(device->createFenceUnique({vk::FenceCreateFlagBits::eSignaled})) {}

std::vector<vkx::SyncObjects> vkx::SyncObjects::createSyncObjects(const Device &device) {
    std::vector<vkx::SyncObjects> objs;
    objs.resize(MAX_FRAMES_IN_FLIGHT);

    std::ranges::generate(objs, [&device]() { return SyncObjects{device}; });

    return objs;
}

template<>
vkx::ShaderUniformVariable<vk::UniqueSampler>::ShaderUniformVariable(vk::UniqueSampler &&variable)
        : variable(std::move(variable)) {}

template<>
vk::DescriptorSetLayoutBinding
vkx::ShaderUniformVariable<vk::UniqueSampler>::createDescriptorSetLayoutBinding(std::uint32_t binding,
                                                                                vk::ShaderStageFlagBits flags) const {
    return vk::DescriptorSetLayoutBinding{ binding, vk::DescriptorType::eCombinedImageSampler, 1, flags };
}

vkx::RendererBase::RendererBase(SDL_Window *window, Profile const &profile)
        : RendererContext(window, profile),
          window(window) {
    VkSurfaceKHR cSurface = nullptr;
    if (SDL_Vulkan_CreateSurface(window, *instance, &cSurface) != SDL_TRUE) {
        throw std::runtime_error("Failure to create C API via SDL2 Vulkan surface.");
    }
    surface = vk::UniqueSurfaceKHR(cSurface, *instance);

    device = vkx::Device{RendererBase::getBestPhysicalDevice(surface, profile), surface, profile};

    createSwapchain();

    vk::DescriptorSetLayoutBinding uboLayoutBinding{
            0,                                  // binding
            vk::DescriptorType::eUniformBuffer, // descriptorType
            1,                                  // descriptorCount
            vk::ShaderStageFlagBits::eVertex,   // stageFlags
            nullptr                             // pImmutableSamplers
    };

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{
            1,                                         // binding
            vk::DescriptorType::eCombinedImageSampler, // descriptorType
            1,                                         // descriptorCount
            vk::ShaderStageFlagBits::eFragment,        // stageFlags
            nullptr                                    // pImmutableSamplers
    };

    vk::DescriptorSetLayoutBinding lightLayoutBinding{
            2,                                  // binding
            vk::DescriptorType::eUniformBuffer, // descriptorType
            1,                                  // descriptorCount
            vk::ShaderStageFlagBits::eFragment, // stageFlags
            nullptr                             // pImmutableSamplers
    };

    vk::DescriptorSetLayoutBinding materialLayoutBinding{
            3,                                  // binding
            vk::DescriptorType::eUniformBuffer, // descriptorType
            1,                                  // descriptorCount
            vk::ShaderStageFlagBits::eFragment, // stageFlags
            nullptr                             // pImmutableSamplers
    };

    std::vector<vk::DescriptorSetLayoutBinding> bindings{
            uboLayoutBinding,
            samplerLayoutBinding,
            lightLayoutBinding,
            materialLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
            {},      // flags
            bindings // binding
    };

    descriptorSetLayout = device->createDescriptorSetLayoutUnique(layoutInfo);

    graphicsPipeline = GraphicsPipeline{device, swapchain.extent, renderPass, descriptorSetLayout};

    drawCommands = device.createDrawCommands(MAX_FRAMES_IN_FLIGHT);

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::FenceCreateInfo fenceInfo{
            vk::FenceCreateFlagBits::eSignaled // flags
    };

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = device->createSemaphoreUnique({});
        renderFinishedSemaphores[i] = device->createSemaphoreUnique({});
        inFlightFences[i] = device->createFenceUnique(fenceInfo);
    }

    syncObjects = SyncObjects::createSyncObjects(device);

    createDescriptorPool();
}

vkx::RendererBase::RendererBase(const SDLWindow &window, const Profile &profile)
    : vkx::RendererContext(window, profile) {
    surface = window.createSurface(instance);

    device = vkx::Device{getBestPhysicalDevice(surface, profile), surface, profile};

    createSwapchain();

    vk::DescriptorSetLayoutBinding uboLayoutBinding{
            0,                                  // binding
            vk::DescriptorType::eUniformBuffer, // descriptorType
            1,                                  // descriptorCount
            vk::ShaderStageFlagBits::eVertex,   // stageFlags
            nullptr                             // pImmutableSamplers
    };

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{
            1,                                         // binding
            vk::DescriptorType::eCombinedImageSampler, // descriptorType
            1,                                         // descriptorCount
            vk::ShaderStageFlagBits::eFragment,        // stageFlags
            nullptr                                    // pImmutableSamplers
    };

    vk::DescriptorSetLayoutBinding lightLayoutBinding{
            2,                                  // binding
            vk::DescriptorType::eUniformBuffer, // descriptorType
            1,                                  // descriptorCount
            vk::ShaderStageFlagBits::eFragment, // stageFlags
            nullptr                             // pImmutableSamplers
    };

    vk::DescriptorSetLayoutBinding materialLayoutBinding{
            3,                                  // binding
            vk::DescriptorType::eUniformBuffer, // descriptorType
            1,                                  // descriptorCount
            vk::ShaderStageFlagBits::eFragment, // stageFlags
            nullptr                             // pImmutableSamplers
    };

    std::vector<vk::DescriptorSetLayoutBinding> bindings{
            uboLayoutBinding,
            samplerLayoutBinding,
            lightLayoutBinding,
            materialLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
            {},      // flags
            bindings // binding
    };

    descriptorSetLayout = device->createDescriptorSetLayoutUnique(layoutInfo);

    graphicsPipeline = GraphicsPipeline{device, swapchain.extent, renderPass, descriptorSetLayout};

    drawCommands = device.createDrawCommands(MAX_FRAMES_IN_FLIGHT);

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::FenceCreateInfo fenceInfo{
            vk::FenceCreateFlagBits::eSignaled // flags
    };

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = device->createSemaphoreUnique({});
        renderFinishedSemaphores[i] = device->createSemaphoreUnique({});
        inFlightFences[i] = device->createFenceUnique(fenceInfo);
    }

    syncObjects = SyncObjects::createSyncObjects(device);

    createDescriptorPool();
}

namespace vkx {
    void RendererBase::recreateSwapchain() {
        int width;
        int height;
        SDL_GetWindowSize(window, &width, &height);
        while (width == 0 || height == 0) {
            SDL_GetWindowSize(window, &width, &height);
            SDL_WaitEvent(nullptr);
        }

        device->waitIdle();

        createSwapchain();
        graphicsPipeline = GraphicsPipeline{device, swapchain.extent, renderPass, descriptorSetLayout};
    }

    void RendererBase::createDescriptorPool() {
        vk::DescriptorPoolSize uniformBufferDescriptor{
                vk::DescriptorType::eUniformBuffer, // type
                MAX_FRAMES_IN_FLIGHT                // descriptorType
        };

        std::array<vk::DescriptorPoolSize, 4> poolSizes{};
        std::ranges::fill(poolSizes, uniformBufferDescriptor);

        poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;

        vk::DescriptorPoolCreateInfo poolInfo{
                {},                   // flags
                MAX_FRAMES_IN_FLIGHT, // maxSets
                poolSizes             // poolSizes
        };

        descriptorPool = device->createDescriptorPoolUnique(poolInfo);
    }

    void RendererBase::createDescriptorSets(
            std::vector<UniformBuffer<MVP>> const &mvpBuffers,
            std::vector<UniformBuffer<DirectionalLight>> const &lightBuffers,
            std::vector<UniformBuffer<Material>> const &materialBuffers,
            Texture const &texture) {
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{
                *descriptorPool, // descriptorPool
                layouts             // setLayouts
        };

        descriptorSets = device->allocateDescriptorSets(allocInfo);

        for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            std::array<vk::WriteDescriptorSet, 4> descriptorWrites{
                    mvpBuffers[i].createWriteDescriptorSet(descriptorSets[i], 0),
                    texture.createWriteDescriptorSet(descriptorSets[i], 1),
                    lightBuffers[i].createWriteDescriptorSet(descriptorSets[i], 2),
                    materialBuffers[i].createWriteDescriptorSet(descriptorSets[i], 3),
            };

            device->updateDescriptorSets(descriptorWrites, {});
        }
    }

    void RendererBase::drawFrame(UniformBuffer<MVP> const &mvpBuffer,
                                 UniformBuffer<DirectionalLight> const &lightBuffer,
                                 UniformBuffer<Material> const &materialBuffer,
                                 Buffer const &vertexBuffer,
                                 Buffer const &indexBuffer,
                                 std::uint32_t indexCount,
                                 std::uint32_t &currentIndexFrame) {
        static_cast<void>(device->waitForFences(*inFlightFences[currentIndexFrame], true, UINT64_MAX));
        auto [result, imageIndex] = swapchain.acquireNextImage(device, imageAvailableSemaphores[currentIndexFrame]);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain();
            return;
        } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swap chain image.");
        }

        mvpBuffer.mapMemory();
        lightBuffer.mapMemory();
        materialBuffer.mapMemory();

        device->resetFences(*inFlightFences[currentIndexFrame]);

        drawCommands[currentIndexFrame].record(*renderPass, *swapchain.framebuffers[imageIndex], swapchain.extent,
                                               *graphicsPipeline.pipeline, *graphicsPipeline.layout,
                                               descriptorSets[currentIndexFrame],
                                               vertexBuffer, indexBuffer, indexCount);

        std::vector<vk::CommandBuffer> commandBuffers{
                static_cast<vk::CommandBuffer>(drawCommands[currentIndexFrame])
        };
        device.submit(commandBuffers, *imageAvailableSemaphores[currentIndexFrame],
                      *renderFinishedSemaphores[currentIndexFrame], *inFlightFences[currentIndexFrame]);

        result = device.present(swapchain, imageIndex, *renderFinishedSemaphores[currentIndexFrame]);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapchain();
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present swapchain image.");
        }

        currentIndexFrame = getCurrentFrameIndex(currentIndexFrame);
    }

    std::uint32_t RendererBase::getCurrentFrameIndex(std::uint32_t frameIndex) const {
        return (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void RendererBase::createSwapchain() {
        swapchain = {device, surface, window, swapchain};

        renderPass = createRenderPass();

        swapchain.createFramebuffers(device, renderPass);
    }

    vk::UniqueRenderPass RendererBase::createRenderPass(vk::AttachmentLoadOp loadOp) const {
        vk::AttachmentDescription colorAttachment{
                {},                               // flags
                swapchain.imageFormat,            // format
                vk::SampleCountFlagBits::e1,      // samples
                loadOp,                           // loadOp
                vk::AttachmentStoreOp::eStore,    // storeOp
                vk::AttachmentLoadOp::eDontCare,  // stencilLoadOp
                vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
                vk::ImageLayout::eUndefined,      // initialLayout
                vk::ImageLayout::ePresentSrcKHR   // finalLayout
        };

        vk::AttachmentReference colorAttachmentRef{
                0,                                       // attachment
                vk::ImageLayout::eColorAttachmentOptimal // layout
        };

        vk::AttachmentDescription depthAttachment{
                {},                                             // flags
                device.findDepthFormat(),                       // format
                vk::SampleCountFlagBits::e1,                    // samples
                vk::AttachmentLoadOp::eClear,                   // loadOp
                vk::AttachmentStoreOp::eDontCare,               // storeOp
                vk::AttachmentLoadOp::eDontCare,                // stencilLoadOp
                vk::AttachmentStoreOp::eDontCare,               // stencilStoreOp
                vk::ImageLayout::eUndefined,                    // initialLayout
                vk::ImageLayout::eDepthStencilAttachmentOptimal // finalLayout
        };

        vk::AttachmentReference depthAttachmentRef{
                1,                                              // attachment
                vk::ImageLayout::eDepthStencilAttachmentOptimal // layout
        };

        vk::SubpassDescription subpass{
                {},                               // flags
                vk::PipelineBindPoint::eGraphics, // pipelineBindPoint
                {},                               // inputAttachments
                colorAttachmentRef,               // colorAttachments
                {},                               // resolveAttachments
                &depthAttachmentRef,              // pDepthStencilAttachment
                {}                                // preserveAttachments
        };

        auto dependencyStageMask =
                vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        auto dependencyAccessMask =
                vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        vk::SubpassDependency dependency{
                VK_SUBPASS_EXTERNAL, // srcSubpass
                0,                   // dstSubpass
                dependencyStageMask, // srcStageMask
                dependencyStageMask, // dstStageMask
                {},                  // srcAccessMask
                dependencyAccessMask // dstAccessMask
        };

        std::vector renderPassAttachments{
                colorAttachment,
                depthAttachment};

        vk::RenderPassCreateInfo renderPassInfo{
                {},                    // flags
                renderPassAttachments, // attachments
                subpass,               // subpasses
                dependency             // dependencies
        };

        return device->createRenderPassUnique(renderPassInfo);
    }

}