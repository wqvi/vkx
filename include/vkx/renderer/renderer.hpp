#pragma once

#include <vkx/renderer/model.hpp>
#include <vkx/renderer/core/bootstrap.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/application.hpp>

namespace vkx {
struct DrawInfoTest {
    const vk::CommandBufferLevel level = vk::CommandBufferLevel::eSecondary;
    const GraphicsPipeline* graphicsPipeline = nullptr;
    const std::vector<const vkx::Mesh*> meshes{};
};

class Renderer {
public:
    RendererBootstrap bootstrap{};
    Device device{};
public:
    Allocator allocator{};
    CommandSubmitter commandSubmitter{};
private:
    Swapchain swapchain{};
    vk::UniqueRenderPass clearRenderPass{};
    vk::UniqueRenderPass loadRenderPass{};

    std::vector<vkx::GraphicsPipeline> pipelines{};

public:
    std::vector<vkx::SyncObjects> syncObjects{};

    std::uint32_t currentFrame = 0;

    std::uint32_t imageIndex = 0;

    bool framebufferResized = false;

    std::vector<vk::CommandBuffer> primaryCommandsBuffers{};
    std::size_t primaryDrawCommandsAmount = 0;
    std::vector<vk::CommandBuffer> secondaryCommandBuffers{};
    std::size_t secondaryDrawCommandsAmount = 0;

public:
    Renderer() = default;

    explicit Renderer(const SDLWindow& window);

    vkx::GraphicsPipeline* attachPipeline(const GraphicsPipelineInformation& pipelineInformation);

    void resized(const SDLWindow& window);

    template <class T, class K>
    auto createMesh(const T& vertices, const K& indices) const {
        return vkx::Mesh{vertices, indices, allocator};
    }

    vkx::Texture createTexture(const std::string& file) const;

    void createDrawCommands(const std::vector<DrawInfoTest>& drawInfos);

    void lazySync(const vkx::SDLWindow& window);

    void uploadDrawCommands(const std::vector<DrawInfoTest>& drawInfos, const SDLWindow& window);

    void lazyUpdate();
};
} // namespace vkx
