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

    template<class T>
    struct ShaderUniformVariable {
        explicit ShaderUniformVariable(T &&variable);

        [[nodiscard]]
        vk::DescriptorSetLayoutBinding
        createDescriptorSetLayoutBinding(std::uint32_t binding, vk::ShaderStageFlagBits flags) const;

        T variable;
    };

    template<class T>
    ShaderUniformVariable<T>::ShaderUniformVariable(T &&variable)
            : variable(std::move(variable)) {}

    template<class T>
    vk::DescriptorSetLayoutBinding
    ShaderUniformVariable<T>::createDescriptorSetLayoutBinding(std::uint32_t binding,
                                                               vk::ShaderStageFlagBits flags) const {
        return vk::DescriptorSetLayoutBinding{binding, vk::DescriptorType::eUniformBuffer, 1, flags};
    }

    class RendererBase : public RendererContext {
    public:
        RendererBase() = default;

        RendererBase(SDLWindow &window, const Profile &profile);

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

        [[nodiscard]]
        std::uint32_t getCurrentFrameIndex() const;

        template<class T>
        auto createBuffers(T const &value = {}) const {
            std::vector<vkx::UniformBuffer<T>> buffers;
            buffers.reserve(MAX_FRAMES_IN_FLIGHT);
            for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                buffers.emplace_back(value, device);
            }
            return buffers;
        }

        [[nodiscard]]
        Mesh allocateMesh(const std::vector<Vertex> &vertices,
                          const std::vector<std::uint32_t> &indices) const;

        [[nodiscard]]
        Texture allocateTexture(const std::string &textureFile) const;

        void waitIdle() const;

    protected:
        SDLWindow *window;
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

        std::vector<SyncObjects> syncObjects;

        std::uint32_t currentFrame = 0;

//        bool framebufferResized = false;

    private:
        void createSwapchain();

        [[nodiscard]]
        vk::UniqueRenderPass createRenderPass(vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) const;

    };
}