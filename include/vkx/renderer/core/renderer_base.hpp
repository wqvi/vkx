#pragma once

#include <SDL2/SDL_video.h>
#include <cstddef>
#include <cstdint>
#include <vkx/camera.hpp>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/texture.hpp>
#include <vulkan/vulkan_core.h>

struct SwapchainInfo {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	explicit SwapchainInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	VkSurfaceFormatKHR chooseSurfaceFormat() const;

	VkPresentModeKHR choosePresentMode() const;

	VkExtent2D chooseExtent(int width, int height) const;

	std::uint32_t getImageCount() const;

	bool complete() const noexcept;
};

struct Queue {
	VkQueue graphics = nullptr;
	VkQueue present = nullptr;
};

struct QueueConfig {
	std::uint32_t graphicsIndex = UINT32_MAX;
	std::uint32_t presentIndex = UINT32_MAX;

	explicit QueueConfig(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	std::vector<VkDeviceQueueCreateInfo> createQueueInfos(float priority) const;

	VkSharingMode getImageSharingMode() const;

	std::vector<std::uint32_t> getContigousValues() const;

	bool complete() const noexcept;
};

class VulkanSwapchain {
private:
	VkSwapchainKHR swapchain;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkFramebuffer> framebuffers;

public:
	VulkanSwapchain() = default;

	VulkanSwapchain(SDL_Window* window, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSwapchainKHR oldSwapchain);

	void destroy() const noexcept;

	void createFramebuffers(VkDevice device, VkRenderPass renderPass);

private:
	static VkSwapchainKHR createSwapchain(const SwapchainInfo& info, const QueueConfig config, SDL_Window* window, VkDevice device, VkSurfaceKHR surface);
};

class VulkanDevice {
private:
	VkPhysicalDevice physicalDevice = nullptr;
	VkPhysicalDeviceProperties properties = {};
	VkDevice device = nullptr;
	Queue queue = {};
	VkCommandPool commandPool = nullptr;
	VmaAllocator allocator = nullptr;

public:
	VulkanDevice() = default;

	explicit VulkanDevice(VkInstance instance, VkSurfaceKHR surface);

	void destroy() const noexcept;

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

	inline VkFormat findDepthFormat() const {
		return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

private:
	static VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

	static VkDevice createDevice(const QueueConfig& queueConfig, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	static VkCommandPool createCommandPool(const QueueConfig& queueConfig, VkDevice device);
};

class VulkanBootstrap {
private:
	SDL_Window* window = nullptr;
	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;
	VulkanDevice device = {};

public:
	VulkanBootstrap() = delete;

	explicit VulkanBootstrap(SDL_Window* window);

	~VulkanBootstrap();

private:
	static VkBool32 debug(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

	static VkInstance initInstance(SDL_Window* window, VkApplicationInfo* applicationInfo);

	static VkSurfaceKHR initSurface(SDL_Window* window, VkInstance instance);
};

namespace vkx {
class RendererBase {
public:
	RendererBase(SDL_Window* window);

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
		       const VertexBuffer& vertexBuffer,
		       const IndexBuffer& indexBuffer, std::uint32_t indexCount,
		       std::uint32_t& currentIndexFrame);

	[[nodiscard]] std::uint32_t getCurrentFrameIndex() const;

	template <class T>
	auto createBuffers(T const& value = {}) const {
		std::vector<vkx::UniformBuffer<T>> buffers;
		buffers.reserve(MAX_FRAMES_IN_FLIGHT);
		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			buffers.emplace_back(value, *device);
		}
		return buffers;
	}

	[[nodiscard]] Mesh
	allocateMesh(const std::vector<Vertex>& vertices,
		     const std::vector<std::uint32_t>& indices) const;

	[[nodiscard]] Texture allocateTexture(const std::string& textureFile) const;

	void waitIdle() const;

	bool framebufferResized = false;

private:
	SDL_Window* window;
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