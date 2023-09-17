#include <vkx/application.hpp>

namespace vkx {
application::application() {
#ifdef DEBUG
	SDL_Log("Hello!");
#endif

	if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC) != 0) {
		throw std::runtime_error(SDL_GetError());
	}

	window = SDL_CreateWindow("VKX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

	if (window == nullptr) {
		throw std::runtime_error(SDL_GetError());
	}

	instance = vkx::VulkanInstance{window};

	commandSubmitter = instance.createCommandSubmitter();

	texture = vkx::Texture{"resources/a.jpg", instance, commandSubmitter};

	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex};

	constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment};

	const vkx::pipeline::GraphicsPipelineInformation graphicsPipelineInformation{
	    "build/shader2D.vert.spv",
	    "build/shader2D.frag.spv",
	    {uboLayoutBinding, samplerLayoutBinding},
	    vkx::Vertex::getBindingDescription(),
	    vkx::Vertex::getAttributeDescriptions(),
	    {sizeof(vkx::MVP)},
	    {&texture}};

	pipeline = instance.createGraphicsPipeline(graphicsPipelineInformation);

}

application::~application() {
	instance.waitIdle();

	texture.destroy();
	pipeline.destroy();
	commandSubmitter.destroy();
	instance.destroy();

	if (window) {
		SDL_DestroyWindow(window);
	}

	SDL_Quit();

#ifdef DEBUG
	SDL_Log("Good bye!");
#endif
}

void application::run() {
	isRunning = true;

	std::uint32_t currentFrame = 0;
	bool framebufferResized = false;

	SDL_ShowWindow(window);
	while (isRunning) {
		poll();
	}

	// cleanup?
}

void application::poll() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		const auto eventType = event.type;

		switch (eventType) {
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_WINDOWEVENT:
			break;
		case SDL_KEYDOWN:
			break;
		case SDL_KEYUP:
			break;
		case SDL_MOUSEMOTION:
			break;
		default:
			break;
		}
	}
}
}
