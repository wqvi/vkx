#pragma once

#include "vkx/renderer/core/device.hpp"
#include "vkx/renderer/uniform_buffer.hpp"
#include <SDL2/SDL_video.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vkx/camera.hpp>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/sync_objects.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vkx/renderer/model.hpp>

namespace vkx {
class RendererBase {
public:
	explicit RendererBase(SDL_Window* window);

	[[nodiscard]]
	std::shared_ptr<vkx::Device> createDevice() const; 

	void recreateSwapchain();

	void createDescriptorPool();

	void createDescriptorSets(
	    const std::vector<UniformBuffer<MVP>>& mvpBuffers,
	    const std::vector<UniformBuffer<DirectionalLight>>& lightBuffers,
	    const std::vector<UniformBuffer<Material>>& materialBuffers,
	    const Texture& texture);

	void drawFrame(const UniformBuffer<MVP>& mvpBuffer,
		       const UniformBuffer<DirectionalLight>& lightBuffer,
		       const UniformBuffer<Material>& materialBuffer,
		       vk::Buffer vertexBuffer,
		       vk::Buffer indexBuffer,
		       std::uint32_t indexCount,
		       std::uint32_t& currentIndexFrame);

	[[nodiscard]] std::uint32_t getCurrentFrameIndex() const;

	template <class T>
	std::vector<vkx::UniformBuffer<T>> createBuffers(T const& value = {}) const {
		std::vector<vkx::UniformBuffer<T>> buffers;
		buffers.reserve(MAX_FRAMES_IN_FLIGHT);
		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			buffers.emplace_back(value, allocator);
		}
		return buffers;
	}

	[[nodiscard]] Mesh
	allocateMesh(const std::vector<Vertex>& vertices,
		     const std::vector<std::uint32_t>& indices) const;

	template <std::size_t T, std::size_t K>
	vkx::Mesh allocateMesh(const std::array<Vertex, T>& vertices, const std::array<std::uint32_t, K>& indices) const {
		return Mesh{vertices, indices, allocator};
	}

	[[nodiscard]] Texture allocateTexture(const std::string& textureFile) const;

	void waitIdle() const;

	bool framebufferResized = false;

private:
	SDL_Window* window;
	vk::UniqueInstance instance;
	vk::UniqueSurfaceKHR surface;
	std::unique_ptr<Device> device;
	std::shared_ptr<Allocator> allocator{};

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

	static vk::PhysicalDevice getBestPhysicalDevice(const vk::UniqueInstance& instance, const vk::UniqueSurfaceKHR& surface);
};
} // namespace vkx
