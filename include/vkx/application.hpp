#pragma once

#include <vkx/renderer/core/bootstrap.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vkx {
class Renderer {
public:
	SDL_Window* window = nullptr;

	RendererBootstrap bootstrap;
	Device device;
	std::shared_ptr<Allocator> allocator;
	std::shared_ptr<CommandSubmitter> commandSubmitter;
	std::shared_ptr<Swapchain> swapchain;
	vk::UniqueRenderPass clearRenderPass;
	vk::UniqueRenderPass loadRenderPass;

	vk::UniqueDescriptorPool descriptorPool;
	vk::UniqueDescriptorPool highlightDescriptorPool;

    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::UniqueDescriptorSetLayout highlightDescriptorSetLayout;

    std::shared_ptr<GraphicsPipeline> graphicsPipeline;
    std::shared_ptr<GraphicsPipeline> highlightGraphicsPipeline;

	std::vector<SyncObjects> syncObjects;

	std::vector<vk::DescriptorSet> descriptorSets;
	std::vector<vk::DescriptorSet> highlightDescriptorSets;

	std::vector<vk::CommandBuffer> drawCommands;
	std::vector<vk::CommandBuffer> secondaryDrawCommands;

	explicit Renderer(SDL_Window* window);

	void recreateSwapchain();

private:
	static vk::UniqueDescriptorSetLayout createShaderDescriptorSetLayout(vk::Device device);

	static vk::UniqueDescriptorSetLayout createHighlightDescriptorSetLayout(vk::Device device);

	static std::vector<vk::VertexInputBindingDescription> getBindingDescription() noexcept;

	static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() noexcept;
};

class SDLWindow {
private:
	struct SDLDeleter {
		void operator()(SDL_Window* ptr) noexcept;
	};

	std::unique_ptr<SDL_Window, SDLDeleter> window;

public:
	explicit SDLWindow(const char* title, int width, int height);

	explicit operator SDL_Window*() const noexcept;
};

struct Scene {
    Scene() = default;

    virtual ~Scene() = default;

    virtual void init(const vkx::Device& device, const std::shared_ptr<vkx::Allocator>& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter) = 0;

    virtual void destroy() = 0;

    virtual void update() = 0;

	virtual void windowResize(int width, int height) = 0;

    virtual void mouseMoved(const SDL_MouseMotionEvent& event) = 0;

    virtual void mousePressed(const SDL_MouseButtonEvent& event) = 0;

    virtual void mouseReleased(const SDL_MouseButtonEvent& event) = 0;

    virtual void keyPressed(const SDL_KeyboardEvent& event) = 0;

    virtual void keyReleased(const SDL_KeyboardEvent& event) = 0;
};

class Application {
private:
	int state = 1;

	SDLWindow window;

	Renderer renderer;

    std::unique_ptr<Scene> scene;

	bool isRunning = true;

	bool framebufferResized = false;

	SDL_Event event{};

public:
	vk::Buffer vertexBuffer;
	vk::Buffer indexBuffer;
	std::uint32_t indexCount;

	vk::Buffer highlightVertexBuffer;
	vk::Buffer highlightIndexBuffer;
	std::uint32_t highlightIndexCount;

	Application();

	Application(const Application&) = delete;

	Application(Application&&) noexcept = default;

	virtual ~Application();

	Application& operator=(const Application& other) = delete;

	Application& operator=(Application&&) noexcept = default;

    void setScene(Scene* newScene);

    void run();

private:
	void handleEvents();

	void handleWindowEvent(const SDL_WindowEvent& event);

	void handleMouseMovedEvent(const SDL_MouseMotionEvent& event);

    void handleMousePressedEvent(const SDL_MouseButtonEvent& event);

    void handleMouseReleasedEvent(const SDL_MouseButtonEvent& event);

    void handleKeyPressedEvent(const SDL_KeyboardEvent& event);

    void handleKeyReleasedEvent(const SDL_KeyboardEvent& event);

	static int SDLInit();
};
} // namespace vkx