#include <SDL2/SDL_video.h>
#include <vkx/application.hpp>

constexpr std::uint32_t chunkDrawCommandAmount = 1;
constexpr std::uint32_t highlightDrawCommandAmount = 1;

constexpr std::uint32_t drawCommandAmount = chunkDrawCommandAmount + highlightDrawCommandAmount;
constexpr std::uint32_t secondaryDrawCommandAmount = 4;

vkx::Renderer::Renderer(SDL_Window* window)
    : window(window),
      bootstrap(window),
      device(bootstrap.createDevice()),
    //   allocator(device.createAllocator()),
    //   commandSubmitter(device.createCommandSubmitter()),
    //   swapchain(device.createSwapchain(window, allocator)),
      clearRenderPass(device.createRenderPass(swapchain->format(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear)),
      loadRenderPass(device.createRenderPass(swapchain->format(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eLoad)) {
	swapchain->createFramebuffers(device, *clearRenderPass);

	constexpr vk::DescriptorPoolSize uniformBufferDescriptor{vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT};
	constexpr vk::DescriptorPoolSize samplerBufferDescriptor{vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT};

	constexpr std::array poolSizes = {uniformBufferDescriptor, samplerBufferDescriptor, uniformBufferDescriptor, uniformBufferDescriptor};

	const vk::DescriptorPoolCreateInfo poolInfo{{}, MAX_FRAMES_IN_FLIGHT, poolSizes};

	descriptorPool = device->createDescriptorPoolUnique(poolInfo);

	constexpr std::array highlightPoolSizes = {uniformBufferDescriptor};

	const vk::DescriptorPoolCreateInfo highlightPoolInfo{{}, MAX_FRAMES_IN_FLIGHT, highlightPoolSizes};

	highlightDescriptorPool = device->createDescriptorPoolUnique(poolInfo);

	descriptorSetLayout = createShaderDescriptorSetLayout(device);

	highlightDescriptorSetLayout = createHighlightDescriptorSetLayout(device);

	vkx::GraphicsPipelineInformation graphicsPipelineInformation{
	    "shader.vert.spv",
	    "shader.frag.spv",
	    *clearRenderPass,
	    *descriptorSetLayout,
	    vkx::Vertex::getBindingDescription(),
	    vkx::Vertex::getAttributeDescriptions()};

	// graphicsPipeline = device.createGraphicsPipeline(graphicsPipelineInformation);

	vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation{
	    "highlight.vert.spv",
	    "highlight.frag.spv",
	    *clearRenderPass,
	    *highlightDescriptorSetLayout,
	    getBindingDescription(),
	    getAttributeDescriptions()};

	// highlightGraphicsPipeline = device.createGraphicsPipeline(highlightGraphicsPipelineInformation);

	syncObjects = device.createSyncObjects();

	drawCommands = commandSubmitter->allocateDrawCommands(drawCommandAmount);
	secondaryDrawCommands = commandSubmitter->allocateSecondaryDrawCommands(secondaryDrawCommandAmount);
}

void vkx::Renderer::recreateSwapchain() {
	int newWidth = 0;
	int newHeight = 0;
	SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
	while (newWidth == 0 || newHeight == 0) {
		SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
		SDL_WaitEvent(nullptr);
	}

	device->waitIdle();

	// swapchain = device.createSwapchain(window, allocator);

	// swapchain->createFramebuffers(device, *clearRenderPass);
}

vk::UniqueDescriptorSetLayout vkx::Renderer::createShaderDescriptorSetLayout(const vkx::Device& device) {
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
	return device->createDescriptorSetLayoutUnique(layoutInfo);
};

vk::UniqueDescriptorSetLayout vkx::Renderer::createHighlightDescriptorSetLayout(const vkx::Device& device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr std::array bindings = {uboLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};
	return device->createDescriptorSetLayoutUnique(layoutInfo);
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

void vkx::SDLWindow::show() const {
	SDL_ShowWindow(window.get());
}

void vkx::SDLWindow::hide() const {
	SDL_HideWindow(window.get());
}

std::pair<int, int> vkx::SDLWindow::getSize() const {
	int width;
	int height;
	SDL_Vulkan_GetDrawableSize(window.get(), &width, &height);

	return std::make_pair(width, height);
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

void vkx::Application::setScene(Scene* newScene) {
	scene.reset(newScene);
}

void vkx::Application::run() {
	std::uint32_t currentFrame = 0;
	SDL_ShowWindow(static_cast<SDL_Window*>(window));

	while (isRunning) {
		// Do stuff
		// scene->update();

		// // Render
		// const auto& syncObject = renderer.syncObjects[currentFrame];
		// syncObject.waitForFence();
		// auto [result, imageIndex] = renderer.swapchain->acquireNextImage(renderer.device, syncObject);

		// if (result == vk::Result::eErrorOutOfDateKHR) {
		// 	renderer.recreateSwapchain();
		// 	continue;
		// } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
		// 	throw std::runtime_error("Failed to acquire next image.");
		// }

		// // mvpBuffer.mapMemory();
		// // lightBuffer.mapMemory();
		// // materialBuffer.mapMemory();
		// // highlightMVPBuffer.mapMemory();

		// syncObject.resetFence();

		// const vkx::DrawInfo chunkDrawInfo = {
		//     imageIndex,
		//     currentFrame,
		//     renderer.swapchain,
		//     renderer.graphicsPipeline,
		//     *renderer.clearRenderPass,
		//     {},
		//     {},
		//     {}};

		// const vkx::DrawInfo highlightDrawInfo = {
		//     imageIndex,
		//     currentFrame,
		//     renderer.swapchain,
		//     renderer.highlightGraphicsPipeline,
		//     *renderer.loadRenderPass,
		//     {},
		//     {},
		//     {}};

		// const vk::CommandBuffer* begin = &renderer.drawCommands[currentFrame * drawCommandAmount];

		// const vk::CommandBuffer* chunkBegin = begin;

		// const vk::CommandBuffer* highlightBegin = chunkBegin + chunkDrawCommandAmount;

		// const vk::CommandBuffer* secondaryBegin = &renderer.secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

		// renderer.commandSubmitter->recordSecondaryDrawCommands(chunkBegin, chunkDrawCommandAmount, secondaryBegin, secondaryDrawCommandAmount, chunkDrawInfo);

		// renderer.commandSubmitter->recordPrimaryDrawCommands(highlightBegin, highlightDrawCommandAmount, highlightDrawInfo);

		// renderer.commandSubmitter->submitDrawCommands(begin, drawCommandAmount, syncObject);

		// renderer.commandSubmitter->presentToSwapchain(*renderer.swapchain, imageIndex, syncObject);

		// if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
		// 	renderer.recreateSwapchain();
		// } else if (result != vk::Result::eSuccess) {
		// 	throw std::runtime_error("Failed to present.");
		// }

		// currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		// handleEvents();
	}
}

void vkx::Application::handleEvents() {
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			isRunning = false;
			break;

		case SDL_WINDOWEVENT:
			handleWindowEvent(event.window);
			break;

		case SDL_MOUSEMOTION:
			handleMouseMovedEvent(event.motion);
			break;

		case SDL_MOUSEBUTTONDOWN:
			handleMousePressedEvent(event.button);
			break;

		case SDL_MOUSEBUTTONUP:
			handleMouseReleasedEvent(event.button);
			break;

		case SDL_KEYDOWN:
			handleKeyPressedEvent(event.key);
			break;

		case SDL_KEYUP:
			handleKeyReleasedEvent(event.key);
			break;

		default:
			break;
		}
	}
}

void vkx::Application::handleWindowEvent(const SDL_WindowEvent& event) {
	if (event.event == SDL_WINDOWEVENT_RESIZED) {
		framebufferResized = true;
		scene->windowResize(event.data1, event.data2);
	}
}

void vkx::Application::handleMouseMovedEvent(const SDL_MouseMotionEvent& event) {
	scene->mouseMoved(event);
}

void vkx::Application::handleMousePressedEvent(const SDL_MouseButtonEvent& event) {
	scene->mousePressed(event);
}

void vkx::Application::handleMouseReleasedEvent(const SDL_MouseButtonEvent& event) {
	scene->mouseReleased(event);
}

void vkx::Application::handleKeyPressedEvent(const SDL_KeyboardEvent& event) {
	scene->keyPressed(event);
}

void vkx::Application::handleKeyReleasedEvent(const SDL_KeyboardEvent& event) {
	scene->keyReleased(event);
}

int vkx::Application::SDLInit() {
	int state = SDL_Init(SDL_INIT_EVERYTHING);

	if (state != 0) {
		throw std::runtime_error(SDL_GetError());
	}

	return state;
}