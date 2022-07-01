#pragma once

#include <macros.hpp>
#include <renderer/core/swapchain.hpp>
#include <renderer/core/pipeline.hpp>
#include <camera.hpp>
#include <renderer/core/commands.hpp>
#include <renderer/core/context.hpp>
#include <renderer/texture.hpp>

constexpr static const std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace vkx {
    struct SyncObjects {
        SyncObjects() = default;

        explicit SyncObjects(const Device &device);

        static std::vector<SyncObjects> createSyncObjects(const Device &device);

        vk::UniqueSemaphore imageAvailableSemaphore;
        vk::UniqueSemaphore renderFinishedSemaphore;
        vk::UniqueFence inFlightFence;
    };

    class RendererBase : public RendererContext {
    public:
        RendererBase() = default;

        RendererBase(SDL_Window *window, Profile const &profile);

        RendererBase(const SDLWindow &window, const Profile &profile);

        void recreateSwapchain();

        void createDescriptorPool();

        void createDescriptorSets(std::vector<UniformBuffer<MVP>> const &mvpBuffers,
                                  std::vector<UniformBuffer<DirectionalLight>> const &lightBuffers,
                                  std::vector<UniformBuffer<Material>> const &materialBuffers,
                                  Texture const &texture);

        void drawFrame(UniformBuffer<MVP> const &mvpBuffer,
                       UniformBuffer<DirectionalLight> const &lightBuffer,
                       UniformBuffer<Material> const &materialBuffer,
                       Buffer const &vertexBuffer,
                       Buffer const &indexBuffer,
                       std::uint32_t indexCount,
                       std::uint32_t &currentIndexFrame);

        [[nodiscard]] std::uint32_t getCurrentFrameIndex(std::uint32_t frameIndex) const;

        template<class T>
        auto createBuffers(T const &value = {}) {
            std::vector<vkx::UniformBuffer<T>> buffers;
            buffers.reserve(MAX_FRAMES_IN_FLIGHT);
            for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                buffers.emplace_back(value, device);
            }
            return buffers;
        }

    protected:
        SDL_Window *window = nullptr;
        vk::UniqueSurfaceKHR surface;
        Device device;

        Swapchain swapchain;

        vk::UniqueRenderPass renderPass;
        vk::UniqueDescriptorSetLayout descriptorSetLayout;

        GraphicsPipeline graphicsPipeline;
        ComputePipeline computePipeline;

        vk::UniqueDescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;

        std::vector<DrawCommand> drawCommands;

        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
        std::vector<vk::UniqueFence> inFlightFences;

        std::vector<SyncObjects> syncObjects;

        std::uint32_t currentFrame = 0;

        bool framebufferResized = false;

    private:
        void createSwapchain();

        [[nodiscard]]
        vk::UniqueRenderPass createRenderPass(vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) const;

    };
}