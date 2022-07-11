#pragma once

#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/camera.hpp>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/context.hpp>
#include <vkx/renderer/texture.hpp>
#include <vkx/renderer/core/sync_objects.hpp>

namespace vkx {

    class RendererBase : public RendererContext {
    public:
        RendererBase() = default;

        RendererBase(std::shared_ptr<SDLWindow> const &window, Profile const &profile);

        void recreateSwapchain();

        void createDescriptorPool();

        void createDescriptorSets(std::vector<UniformBuffer<MVP>> const &mvpBuffers,
                                  std::vector<UniformBuffer<DirectionalLight>> const &lightBuffers,
                                  std::vector<UniformBuffer<Material>> const &materialBuffers,
                                  Texture const &texture);

        void drawFrame(UniformBuffer<MVP> const &mvpBuffer,
                       UniformBuffer<DirectionalLight> const &lightBuffer,
                       UniformBuffer<Material> const &materialBuffer,
                       VertexBuffer const &vertexBuffer,
                       IndexBuffer const &indexBuffer,
                       std::uint32_t indexCount,
                       std::uint32_t &currentIndexFrame);

        [[nodiscard]]
        std::uint32_t getCurrentFrameIndex() const;

        template<class T>
        auto createBuffers(T const &value = {}) const {
            std::vector<vkx::UniformBuffer<T>> buffers;
            buffers.reserve(MAX_FRAMES_IN_FLIGHT);
            for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                buffers.emplace_back(value, *device);
            }
            return buffers;
        }

        [[nodiscard]]
        Mesh allocateMesh(const std::vector<Vertex> &vertices,
                          const std::vector<std::uint32_t> &indices) const;

        [[nodiscard]]
        Texture allocateTexture(const std::string &textureFile) const;

        void waitIdle() const;

    private:
        std::weak_ptr<SDLWindow> window;
        vk::UniqueSurfaceKHR surface;
        std::unique_ptr<Device> device;

        Swapchain swapchain;

        vk::UniqueRenderPass renderPass;
        vk::UniqueDescriptorSetLayout descriptorSetLayout;

        GraphicsPipeline graphicsPipeline;
        ComputePipeline computePipeline;

        vk::UniqueDescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;

        std::vector<DrawCommand> drawCommands;

        std::vector<SyncObjects> syncObjects;

        std::uint32_t currentFrame = 0;

        void createSwapchain();

        [[nodiscard]]
        vk::UniqueRenderPass createRenderPass(vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) const;

    };
}