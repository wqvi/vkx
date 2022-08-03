#pragma once

#include <SDL2/SDL_video.h>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vkx/camera.hpp>
#include <vkx/renderer/core/commands.hpp>
#include <vkx/renderer/core/pipeline.hpp>
#include <vkx/renderer/core/swapchain.hpp>
#include <vkx/renderer/core/sync_objects.hpp>
#include <vkx/renderer/texture.hpp>
#include <vulkan/vulkan_core.h>

struct Vertex {
	glm::vec2 pos;

	static constexpr VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {
			.binding = 0,
			.stride = 0,
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};

		return bindingDescription;
	}

	static constexpr std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions() {
		VkVertexInputAttributeDescription posDescription = {
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, pos)
		};

		return {posDescription};
	}
};

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

struct GraphicsPipelineInfo {
	std::string vertexFile = "\0";
	std::string fragmentFile = "\0";
	VkExtent2D extent = {};
	VkRenderPass renderPass = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
};

class VulkanBootstrap;

class VulkanDevice;

class VulkanSwapchain;

class VulkanGraphicsPipeline;

class VulkanDevice {
private:
	VkSurfaceKHR surface = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkPhysicalDeviceProperties properties = {};
	VkDevice device = nullptr;
	Queue queue = {};
	VkCommandPool commandPool = nullptr;
	VmaAllocator allocator = nullptr;

public:
	VulkanDevice() = default;

	explicit VulkanDevice(VkInstance instance, VkSurfaceKHR surface);

	explicit operator VkDevice() const;

	explicit operator VkPhysicalDevice() const;

	void destroy() const noexcept;

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

	inline VkFormat findDepthFormat() const {
		return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

	VmaAllocation allocateImage(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage* image) const;

	VmaAllocation allocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer) const;

	VmaAllocator getAllocator() const noexcept;

	VkRenderPass createRenderPass(VkFormat format, VkAttachmentLoadOp loadOp) const;

	VulkanSwapchain createSwapchain(SDL_Window* window) const;

	VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const;

	VulkanGraphicsPipeline createGraphicsPipeline(const GraphicsPipelineInfo& info) const;

private:
	static VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

	static VkDevice createDevice(const QueueConfig& queueConfig, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	static VkCommandPool createCommandPool(const QueueConfig& queueConfig, VkDevice device);
};

class VulkanSwapchain {
private:
	VkDevice device = nullptr;
	VmaAllocator allocator = nullptr;
	VkSwapchainKHR swapchain = nullptr;
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D extent = {};
	std::vector<VkImage> images = {};
	std::vector<VkImageView> imageViews = {};

	VkImage depthImage = nullptr;
	VmaAllocation depthImageAllocation = nullptr;
	VkImageView depthImageView = nullptr;

	std::vector<VkFramebuffer> framebuffers = {};

public:
	VulkanSwapchain() = default;

	VulkanSwapchain(SDL_Window* window, const VulkanDevice& device, VkSurfaceKHR surface, VkSwapchainKHR oldSwapchain);

	void destroy() const noexcept;

	void createFramebuffers(VkDevice device, VkRenderPass renderPass);

	VkFormat getImageFormat() const noexcept;

	const VkExtent2D& getExtent() const noexcept;

private:
	static VkSwapchainKHR createSwapchain(const SwapchainInfo& info, const QueueConfig config, SDL_Window* window, VkDevice device, VkSurfaceKHR surface);
};

class VulkanGraphicsPipeline {
private:
	VkDevice device = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkPipeline pipeline = nullptr;

public:
	VulkanGraphicsPipeline() = default;

	VulkanGraphicsPipeline(VkDevice device, const GraphicsPipelineInfo& info);

	void destroy() const noexcept;
};

struct SyncObjects {
	VkSemaphore imageAvailableSemaphore = nullptr;
	VkSemaphore renderFinishedSemaphore = nullptr;
	VkFence inFlightFence = nullptr;

	SyncObjects() = default;

	SyncObjects(VkDevice device);

	void destroy(VkDevice device) const noexcept;

	static std::vector<SyncObjects> createSyncObjects(VkDevice device);
};

class VulkanBootstrap {
private:
	SDL_Window* window = nullptr;
	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;

public:
	VulkanBootstrap() = delete;

	explicit VulkanBootstrap(SDL_Window* window);

	~VulkanBootstrap();

	VulkanDevice createDevice() const;

private:
	static VkBool32 debug(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

	static VkInstance initInstance(SDL_Window* window, VkApplicationInfo* applicationInfo);

	static VkSurfaceKHR initSurface(SDL_Window* window, VkInstance instance);

	static VulkanSwapchain initSwapchain();
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
