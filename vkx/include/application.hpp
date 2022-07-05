//
// Created by december on 6/21/22.
//

#pragma once

#include <renderer/core/renderer_base.hpp>
#include <scene.hpp>
#include <renderer/uniform_buffer.hpp>
#include <renderer/model.hpp>

namespace vkx {
    struct ApplicationConfig {
        const char *title = "VKX Application";
        int windowWidth = 640;
        int windowHeight = 360;
        glm::f32 fov = 70.0f;
    };

    class Application {
    public:
        Application() = delete;

        explicit Application(const ApplicationConfig &config);

        ~Application();

        void run();

        void setScene(Scene *newScene);

    private:
        void pollEvents(SDL_Event *event);

        void handleKeyPressedEvent(const SDL_KeyboardEvent &event);

        void handleKeyReleasedEvent(const SDL_KeyboardEvent &event);

        void handleMouseMovedEvent(const SDL_MouseMotionEvent &event);

        ApplicationConfig config;

        SDLWindow window;

        bool isRunning = false;

        RendererBase renderer;

        // Scene must be after renderer base due to it having its resources needing to be cleaned up prior to
        // the renderer cleaning up its own resources
        std::unique_ptr<Scene> scene = nullptr;

    public:
        // TODO Clear violation right here but only to try to get a working application for the time being
        // TODO it is just temporary as there will be a way to attach assets to be rendered
        Model *model;

        std::vector<vkx::UniformBuffer<vkx::MVP>> mvpBuffers;
        std::vector<vkx::UniformBuffer<vkx::DirectionalLight>> lightBuffers;
        std::vector<vkx::UniformBuffer<vkx::Material>> materialBuffers;
    };
}