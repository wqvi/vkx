#include "vkx/renderer/core/device.hpp"
#include "vkx/renderer/core/renderer_types.hpp"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_vulkan.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vk_mem_alloc.h>
#include <vkx/renderer/core/renderer_base.hpp>

#include <iostream>
#include <vkx/debug.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/uniform_buffer.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

vkx::RendererBase::RendererBase(SDL_Window* window) : window(window) {
	constexpr vk::ApplicationInfo applicationInfo(
	    "Star explorer",
	    VK_MAKE_VERSION(0, 0, 1),
	    "VKX",
	    VK_MAKE_VERSION(0, 0, 1),
	    VK_API_VERSION_1_0);

	std::uint32_t count = 0;
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}

	std::vector<const char*> extensions(count);
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}

#ifdef DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

#ifdef DEBUG
	constexpr std::array layers{
	    "VK_LAYER_KHRONOS_validation"};
#elif RELEASE
	constexpr std::array<const char*, 0> layers{};
#endif

	const vk::InstanceCreateInfo instanceCreateInfo(
	    {},
	    &applicationInfo,
	    layers,
	    extensions);

#ifdef DEBUG
	constexpr auto messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	constexpr auto messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

	const vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo(
	    {},
	    messageSeverity,
	    messageType,
	    vkDebugCallback,
	    nullptr);

	vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> structureChain{
	    instanceCreateInfo,
	    debugUtilsMessengerCreateInfo};

	instance = vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
	instance = vk::createInstanceUnique(instanceCreateInfo);
#endif

	VkSurfaceKHR cSurface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, *instance, &cSurface) != SDL_TRUE) {
		throw std::runtime_error(SDL_GetError());
	}
	surface = vk::UniqueSurfaceKHR(cSurface, *instance);

	const auto physicalDevice = getBestPhysicalDevice(instance, surface);
	device = std::make_unique<vkx::Device>(instance, physicalDevice, surface);

	allocator = device->createAllocator(*instance);

	createSwapchain();

	const vk::DescriptorSetLayoutBinding uboLayoutBinding(
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr);

	const vk::DescriptorSetLayoutBinding samplerLayoutBinding(
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr);

	const vk::DescriptorSetLayoutBinding lightLayoutBinding(
	    2,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr);

	const vk::DescriptorSetLayoutBinding materialLayoutBinding(
	    3,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr);

	const auto bindings = {uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings);

	descriptorSetLayout = (*device)->createDescriptorSetLayoutUnique(layoutInfo);

	graphicsPipeline = GraphicsPipeline(*device, swapchain.extent, renderPass, descriptorSetLayout);

	drawCommands = device->createDrawCommands(MAX_FRAMES_IN_FLIGHT);

	syncObjects = SyncObjects::createSyncObjects(*device);

	createDescriptorPool();
}

vkx::Device vkx::RendererBase::createDevice() const {
	const auto physicalDevice = getBestPhysicalDevice(instance, surface);
	return vkx::Device(instance, physicalDevice, surface);
}

void vkx::RendererBase::recreateSwapchain() {
	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);
	while (width == 0 || height == 0) {
		SDL_Vulkan_GetDrawableSize(window, &width, &height);
		SDL_WaitEvent(nullptr);
	}

	(*device)->waitIdle();

	createSwapchain();
	graphicsPipeline = vkx::GraphicsPipeline(*device, swapchain.extent, renderPass, descriptorSetLayout);
}

void vkx::RendererBase::createDescriptorPool() {
	constexpr vk::DescriptorPoolSize uniformBufferDescriptor(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT);
	constexpr vk::DescriptorPoolSize samplerBufferDescriptor(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT);

	constexpr std::array poolSizes{uniformBufferDescriptor, samplerBufferDescriptor, uniformBufferDescriptor, uniformBufferDescriptor};

	const vk::DescriptorPoolCreateInfo poolInfo({}, MAX_FRAMES_IN_FLIGHT, poolSizes);

	descriptorPool = (*device)->createDescriptorPoolUnique(poolInfo);
}

void vkx::RendererBase::createDescriptorSets(const std::vector<UniformBuffer<MVP>>& mvpBuffers, const std::vector<UniformBuffer<DirectionalLight>>& lightBuffers, const std::vector<UniformBuffer<Material>>& materialBuffers, const Texture& texture) {
	const std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo{*descriptorPool, layouts};

	descriptorSets = (*device)->allocateDescriptorSets(allocInfo);

	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		const std::array descriptorWrites{
		    mvpBuffers[i].createWriteDescriptorSet(descriptorSets[i], 0),
		    texture.createWriteDescriptorSet(descriptorSets[i], 1),
		    lightBuffers[i].createWriteDescriptorSet(descriptorSets[i], 2),
		    materialBuffers[i].createWriteDescriptorSet(descriptorSets[i], 3),
		};

		(*device)->updateDescriptorSets(descriptorWrites, {});
	}
}

void vkx::RendererBase::drawFrame(const UniformBuffer<MVP>& mvpBuffer, const UniformBuffer<DirectionalLight>& lightBuffer, const UniformBuffer<Material>& materialBuffer, vk::Buffer vertexBuffer, vk::Buffer indexBuffer, std::uint32_t indexCount, std::uint32_t& currentIndexFrame) {
	static_cast<void>((*device)->waitForFences(*syncObjects[currentIndexFrame].inFlightFence, true, UINT64_MAX));
	auto [result, imageIndex] = swapchain.acquireNextImage(*device, syncObjects[currentIndexFrame].imageAvailableSemaphore);

	if (result == vk::Result::eErrorOutOfDateKHR) {
		recreateSwapchain();
		return;
	} else if (result != vk::Result::eSuccess &&
		   result != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("Failed to acquire next image.");
	}

	mvpBuffer.mapMemory();
	lightBuffer.mapMemory();
	materialBuffer.mapMemory();

	(*device)->resetFences(*syncObjects[currentIndexFrame].inFlightFence);

	drawCommands[currentIndexFrame].record(
	    *renderPass, *swapchain.framebuffers[imageIndex], swapchain.extent,
	    *graphicsPipeline.pipeline, *graphicsPipeline.layout,
	    descriptorSets[currentIndexFrame], vertexBuffer, indexBuffer, indexCount);

	std::vector<vk::CommandBuffer> commandBuffers{static_cast<vk::CommandBuffer>(drawCommands[currentIndexFrame])};
	device->submit(commandBuffers, *syncObjects[currentIndexFrame].imageAvailableSemaphore, *syncObjects[currentIndexFrame].renderFinishedSemaphore, *syncObjects[currentIndexFrame].inFlightFence);

	result = device->present(swapchain, imageIndex, *syncObjects[currentIndexFrame].renderFinishedSemaphore);

	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapchain();
	} else if (result != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to present.");
	}

	currentIndexFrame = (currentIndexFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

std::uint32_t vkx::RendererBase::getCurrentFrameIndex() const {
	return currentFrame;
}

void vkx::RendererBase::createSwapchain() {
	swapchain = vkx::Swapchain(*device, surface, window, allocator);

	renderPass = createRenderPass();

	swapchain.createFramebuffers(*device, renderPass);
}

vk::UniqueRenderPass vkx::RendererBase::createRenderPass(vk::AttachmentLoadOp loadOp) const {
	const vk::AttachmentDescription colorAttachment(
	    {},
	    swapchain.imageFormat,
	    vk::SampleCountFlagBits::e1,
	    loadOp,
	    vk::AttachmentStoreOp::eStore,
	    vk::AttachmentLoadOp::eDontCare,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::ImageLayout::eUndefined,
	    vk::ImageLayout::ePresentSrcKHR);

	const vk::AttachmentReference colorAttachmentRef(
	    0,
	    vk::ImageLayout::eColorAttachmentOptimal);

	const vk::AttachmentDescription depthAttachment(
	    {},
	    device->findDepthFormat(),
	    vk::SampleCountFlagBits::e1,
	    vk::AttachmentLoadOp::eClear,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::AttachmentLoadOp::eDontCare,
	    vk::AttachmentStoreOp::eDontCare,
	    vk::ImageLayout::eUndefined,
	    vk::ImageLayout::eDepthStencilAttachmentOptimal);

	const vk::AttachmentReference depthAttachmentRef(
	    1,
	    vk::ImageLayout::eDepthStencilAttachmentOptimal);

	const vk::SubpassDescription subpass(
	    {},
	    vk::PipelineBindPoint::eGraphics,
	    {},
	    colorAttachmentRef,
	    {},
	    &depthAttachmentRef,
	    {});

	constexpr auto dependencyStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	constexpr auto dependencyAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	const vk::SubpassDependency dependency(
	    VK_SUBPASS_EXTERNAL,
	    0,
	    dependencyStageMask,
	    dependencyStageMask,
	    {},
	    dependencyAccessMask);

	const auto renderPassAttachments = {colorAttachment, depthAttachment};

	const vk::RenderPassCreateInfo renderPassInfo(
	    {},
	    renderPassAttachments,
	    subpass,
	    dependency);

	return (*device)->createRenderPassUnique(renderPassInfo);
}

vkx::Mesh vkx::RendererBase::allocateMesh(
    const std::vector<Vertex>& vertices,
    const std::vector<std::uint32_t>& indices) const {
	return vkx::Mesh{vertices, indices, *device, allocator};
}

vkx::Texture
vkx::RendererBase::allocateTexture(const std::string& textureFile) const {
	return vkx::Texture{textureFile, *device, allocator};
}

void vkx::RendererBase::waitIdle() const { (*device)->waitIdle(); }

vk::PhysicalDevice vkx::RendererBase::getBestPhysicalDevice(const vk::UniqueInstance& instance, const vk::UniqueSurfaceKHR& surface) {
	const auto physicalDevices = instance->enumeratePhysicalDevices();

	vk::PhysicalDevice physicalDevice = nullptr;
	std::uint32_t bestRating = 0;
	for (const auto& pDevice : physicalDevices) {
		std::uint32_t rating = 0;

		const QueueConfig indices{pDevice, surface};
		if (indices.isComplete()) {
			rating++;
		}

		const SwapchainInfo info{pDevice, surface};
		if (info.isComplete()) {
			rating++;
		}

		if (pDevice.getFeatures().samplerAnisotropy) {
			rating++;
		}

		if (rating > bestRating) {
			bestRating = rating;
			physicalDevice = pDevice;
		}
	}

	if (!static_cast<bool>(physicalDevice)) {
		throw std::runtime_error("Failed to find suitable GPU to use.");
	}

	return physicalDevice;
}