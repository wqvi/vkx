#pragma once

#include <vkx/camera.hpp>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/texture.hpp>

namespace vkx {
class RendererBase {
public:
  RendererBase() = default;

  RendererBase(SDL_Window *window);

  void recreateSwapchain();

  void createDescriptorPool();

  void createDescriptorSets(
      std::vector<UniformBuffer<MVP>> const &mvpBuffers,
      std::vector<UniformBuffer<DirectionalLight>> const &lightBuffers,
      std::vector<UniformBuffer<Material>> const &materialBuffers,
      Texture const &texture);

  void drawFrame(UniformBuffer<MVP> const &mvpBuffer,
                 UniformBuffer<DirectionalLight> const &lightBuffer,
                 UniformBuffer<Material> const &materialBuffer,
                 VertexBuffer const &vertexBuffer,
                 IndexBuffer const &indexBuffer, std::uint32_t indexCount,
                 std::uint32_t &currentIndexFrame);

  [[nodiscard]] std::uint32_t getCurrentFrameIndex() const;

  template <class T> auto createBuffers(T const &value = {}) const {
    std::vector<vkx::UniformBuffer<T>> buffers;
    buffers.reserve(MAX_FRAMES_IN_FLIGHT);
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      buffers.emplace_back(value, *device);
    }
    return buffers;
  }

  [[nodiscard]] Mesh
  allocateMesh(const std::vector<Vertex> &vertices,
               const std::vector<std::uint32_t> &indices) const;

  [[nodiscard]] Texture allocateTexture(const std::string &textureFile) const;

  void waitIdle() const;

  bool framebufferResized = false;

private:
  SDL_Window *window;
  vk::UniqueInstance instance;
  vk::UniqueSurfaceKHR surface;
  std::unique_ptr<Device> device;

  Swapchain swapchain;

  vk::UniqueRenderPass renderPass;
  vk::UniqueDescriptorSetLayout descriptorSetLayout;

  GraphicsPipeline graphicsPipeline;

  vk::UniqueDescriptorPool descriptorPool;
  std::vector<vk::DescriptorSet> descriptorSets;

  std::vector<DrawCommand> drawCommands;

  std::vector<SyncObjects> syncObjects;

  std::uint32_t currentFrame = 0;

  void createSwapchain();

  [[nodiscard]] vk::UniqueRenderPass createRenderPass(
      vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) const;
};
} // namespace vkx