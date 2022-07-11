//
// Created by december on 6/21/22.
//

#pragma once

#include <vkx/renderer/core/renderer_base.hpp>
#include <vkx/scene.hpp>
#include <vkx/renderer/uniform_buffer.hpp>
#include <vkx/renderer/model.hpp>

namespace vkx {
    struct GlobalConfiguration {
        // All values are defaulted

        char const *title = "VKX Application";
        int windowWidth = 640;
        int windowHeight = 360;
        glm::f32 fieldOfVision = 70.0f;
    };

    class Application {
    public:
        Application() = delete;

        explicit Application(const GlobalConfiguration &config);

        ~Application();

        void run();

        void setScene(Scene *newScene);

    private:
        void pollEvents(SDL_Event *event);

        void handleKeyPressedEvent(SDL_KeyboardEvent const &event);

        void handleKeyReleasedEvent(SDL_KeyboardEvent const &event);

        void handleMouseMovedEvent(SDL_MouseMotionEvent const &event);

        GlobalConfiguration config;

        std::shared_ptr<SDLWindow> window;

        bool isRunning = false;

        RendererBase renderer;

        // Scene must be after renderer base due to it having its resources needing to be cleaned up prior to
        // the renderer cleaning up its own resources
        std::unique_ptr<Scene> scene = nullptr;

    public:
        // TODO Clear violation right here but only to try to get a working application for the time being
        // TODO it is just temporary as there will be a way to attach assets to be rendered
        Model *model = nullptr;

        std::vector<vkx::UniformBuffer<vkx::MVP>> mvpBuffers;
        std::vector<vkx::UniformBuffer<vkx::DirectionalLight>> lightBuffers;
        std::vector<vkx::UniformBuffer<vkx::Material>> materialBuffers;
    };
}