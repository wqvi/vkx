//
// Created by december on 6/21/22.
//

#pragma once

#include <renderer/core/renderer_base.hpp>
#include <scene.hpp>

namespace vkx {
    class Scene;

    struct ApplicationConfig {
        const char *title = "VKX Application";
        int windowWidth = 640;
        int windowHeight = 360;
        glm::f32 fov = 70.0f;
    };

    class SDLError : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override;
    };

    class VulkanError : public std::exception {
    public:
        explicit VulkanError(const char *message);

        explicit VulkanError(vk::Result result);

        [[nodiscard]] const char *what() const noexcept override;

    private:
        std::string message;
    };

    // A SDL window wrapper class
    // It has unique ownership over the pointer
    class SDLWindow {
        // Helper class that default initializes thus making the construction of the
        // managed pointer much simpler looking
        // This is honestly syntactic constructor sugar in the source files
        struct SDL_Deleter {
            void operator()(SDL_Window *ptr) const {
                SDL_DestroyWindow(ptr);
            }
        };

    public:
        using EventFun = void (*)(SDL_WindowEvent *);

        SDLWindow() = default;

        // Creates a window with the information from the config
        explicit SDLWindow(const ApplicationConfig &config);

        void show() const noexcept;

        void hide() const noexcept;

        [[nodiscard]] std::pair<int, int> getSize() const noexcept;

        [[nodiscard]] int getWidth() const noexcept;

        [[nodiscard]] int getHeight() const noexcept;

        void pollWindowEvent(const SDL_WindowEvent &event);

        void handleResizeEvent(const SDL_WindowEvent &event);

        [[nodiscard]] bool isResized() const noexcept;

        [[nodiscard]] vk::UniqueSurfaceKHR createSurface(const vk::UniqueInstance &instance) const;

    private:
        bool framebufferResized = false;

        std::unique_ptr<SDL_Window, SDL_Deleter> window;
    };

    class Application {
    public:
        Application() = delete;

        explicit Application(const ApplicationConfig &config);

        ~Application();

        void run();

    private:
        void pollEvents(SDL_Event *event);

        void handleKeyPressedEvent(const SDL_KeyboardEvent &event);

        void handleKeyReleasedEvent(const SDL_KeyboardEvent &event);

        void handleMouseMovedEvent(const SDL_MouseMotionEvent &event);

        SDLWindow window;

        Scene *scene = nullptr;

        bool isRunning = false;

        RendererBase renderer;
    };

    class Scene {
    public:
        Scene() = default;

        virtual ~Scene() = default;

        virtual void init(const vkx::ApplicationConfig &config, const vkx::Application *data) = 0;

        virtual void update() = 0;

        virtual void physics(float deltaTime) = 0;

        virtual void destroy() noexcept = 0;

        virtual void onKeyPress() = 0;

        virtual void onKeyRelease() = 0;

        virtual void onMouseMove() = 0;
    };
}