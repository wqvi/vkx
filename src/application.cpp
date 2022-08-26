#include <vkx/application.hpp>
#include <vulkan/vulkan_handles.hpp>

vkx::Renderer::Renderer(SDL_Window* window)
    : window(window),
      bootstrap(window),
      device(bootstrap.createDevice()),
      allocator(device.createAllocator()),
      commandSubmitter(device.createCommandSubmitter()),
      swapchain(device.createSwapchain(window, allocator)),
      clearRenderPass(device.createRenderPass(swapchain->imageFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear)),
      loadRenderPass(device.createRenderPass(swapchain->imageFormat, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eLoad)) {
	swapchain->createFramebuffers(*device, *clearRenderPass);

	constexpr vk::DescriptorPoolSize uniformBufferDescriptor{vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT};
	constexpr vk::DescriptorPoolSize samplerBufferDescriptor{vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT};

	constexpr std::array poolSizes = {uniformBufferDescriptor, samplerBufferDescriptor, uniformBufferDescriptor, uniformBufferDescriptor};

	const vk::DescriptorPoolCreateInfo poolInfo{{}, MAX_FRAMES_IN_FLIGHT, poolSizes};

	descriptorPool = device->createDescriptorPoolUnique(poolInfo);

	constexpr std::array highlightPoolSizes = {uniformBufferDescriptor};

	const vk::DescriptorPoolCreateInfo highlightPoolInfo{{}, MAX_FRAMES_IN_FLIGHT, highlightPoolSizes};

	highlightDescriptorPool = device->createDescriptorPoolUnique(poolInfo);

	descriptorSetLayout = createShaderDescriptorSetLayout(static_cast<vk::Device>(*device));

	highlightDescriptorSetLayout = createHighlightDescriptorSetLayout(static_cast<vk::Device>(*device));

	vkx::GraphicsPipelineInformation graphicsPipelineInformation{
	    "shader.vert.spv",
	    "shader.frag.spv",
	    swapchain->extent,
	    *clearRenderPass,
	    *descriptorSetLayout,
	    vkx::Vertex::getBindingDescription(),
	    vkx::Vertex::getAttributeDescriptions()};

	graphicsPipeline = device.createGraphicsPipeline(graphicsPipelineInformation);

	vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation{
	    "highlight.vert.spv",
	    "highlight.frag.spv",
	    swapchain->extent,
	    *clearRenderPass,
	    *highlightDescriptorSetLayout,
	    getBindingDescription(),
	    getAttributeDescriptions()};

	highlightGraphicsPipeline = device.createGraphicsPipeline(highlightGraphicsPipelineInformation);
}

vk::UniqueDescriptorSetLayout vkx::Renderer::createShaderDescriptorSetLayout(vk::Device device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding lightLayoutBinding{
	    2,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding materialLayoutBinding{
	    3,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr std::array bindings = {uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};
	return device.createDescriptorSetLayoutUnique(layoutInfo);
};

vk::UniqueDescriptorSetLayout vkx::Renderer::createHighlightDescriptorSetLayout(vk::Device device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr std::array bindings = {uboLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};
	return device.createDescriptorSetLayoutUnique(layoutInfo);
}

std::vector<vk::VertexInputBindingDescription> vkx::Renderer::getBindingDescription() noexcept {
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};

	bindingDescriptions.push_back({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex});

	return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription> vkx::Renderer::getAttributeDescriptions() noexcept {
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({0, 0, vk::Format::eR32G32B32Sfloat, 0});

	return attributeDescriptions;
}

void vkx::SDLWindow::SDLDeleter::operator()(SDL_Window* ptr) noexcept {
	if (ptr) {
		SDL_DestroyWindow(ptr);
	}
}

vkx::SDLWindow::SDLWindow(const char* title, int width, int height)
    : window(std::unique_ptr<SDL_Window, vkx::SDLWindow::SDLDeleter>(SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN))) {
	if (window == nullptr) {
		throw std::runtime_error(SDL_GetError());
	}
}

vkx::SDLWindow::operator SDL_Window*() const noexcept {
	return window.get();
}

vkx::Application::Application()
    : state(SDLInit()), window("vkx", 640, 360), renderer(static_cast<SDL_Window*>(window)) {
	if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
		throw std::runtime_error(SDL_GetError());
	}
}

vkx::Application::~Application() {
	if (state == 0) {
		SDL_Quit();
	}
}

int vkx::Application::SDLInit() {
	int state = SDL_Init(SDL_INIT_EVERYTHING);

	if (state != 0) {
		throw std::runtime_error(SDL_GetError());
	}

	return state;
}