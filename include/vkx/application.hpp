#pragma once

#include <vkx/renderer/renderer.hpp>
#include <vkx/renderer/swapchain.hpp>
#include <vkx/renderer/pipeline.hpp>
#include <vkx/renderer/commands.hpp>
#include <vkx/renderer/texture.hpp>

namespace vkx {
class application {
private:
	bool isRunning;
	vkx::VulkanInstance instance;
	vkx::CommandSubmitter commandSubmitter;
	vkx::Texture texture;
	// yea there needs to be more obviously but for now
	vkx::pipeline::GraphicsPipeline pipeline;

public:
	SDL_Window* window;

	application();

	~application();

	void run();

	void poll();
};
}
