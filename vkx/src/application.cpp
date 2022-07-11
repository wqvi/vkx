//
// Created by december on 6/21/22.
//

#include <vkx/application.hpp>

vkx::Application::Application(const vkx::GlobalConfiguration &configuration)
    : config(configuration) {
    int sdlErrorCode = SDL_Init(SDL_INIT_EVERYTHING);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    window = std::make_shared<SDLWindow>(configuration.title, configuration.windowWidth, configuration.windowHeight);

    sdlErrorCode = SDL_ShowCursor(SDL_DISABLE);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    sdlErrorCode = SDL_SetRelativeMouseMode(SDL_TRUE);
    if (sdlErrorCode < 0) {
        throw std::system_error(std::error_code(sdlErrorCode, std::generic_category()), SDL_GetError());
    }

    renderer = vkx::RendererBase{window, vkx::Profile{}};
}

vkx::Application::~Application() {
    if (scene != nullptr) scene->destroy();
    SDL_Quit();
}

void vkx::Application::run() {
    isRunning = true;

    window->show();

    // Declared outside the loop, so it is only initialized once on our side
    SDL_Event event{};

    auto currentFrame = renderer.getCurrentFrameIndex();

    std::chrono::system_clock::time_point lastTime = std::chrono::system_clock::now();
    while (isRunning) {
        // Prep delta time
        auto currentTime = std::chrono::system_clock::now();
        auto deltaTime = std::chrono::duration<float, std::chrono::milliseconds::period>(
                currentTime - lastTime).count();

        // Populate uniforms
        scene->update();
        scene->physics(deltaTime);

        // Query next image

        // Submit draw commands

        // Present queues

        // TODO this hurts my soul
        auto &mvpBuffer = mvpBuffers[currentFrame];
        mvpBuffer->model = model->getModelMatrix();
        mvpBuffer->view = scene->getCamera().viewMatrix();
        mvpBuffer->proj = static_cast<glm::mat4>(scene->getViewport());

        auto &lightBuffer = lightBuffers[currentFrame];
        lightBuffer->position = glm::vec3(1.0f, 3.0f, 1.0f);
        lightBuffer->eyePosition = scene->getCamera().position;
        lightBuffer->ambientColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);
        lightBuffer->diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
        lightBuffer->specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
        lightBuffer->constant = 1.0f;
        lightBuffer->linear = 0.09f;
        lightBuffer->quadratic = 0.032f;

        auto &materialBuffer = materialBuffers[currentFrame];
        materialBuffer->specularColor = model->material.specularColor;
        materialBuffer->shininess = model->material.shininess;

        // TODO whatever this is lol
        renderer.drawFrame(mvpBuffer,
                           lightBuffer,
                           materialBuffer,
                           model->mesh.vertexBuffer,
                           model->mesh.indexBuffer,
                           static_cast<std::uint32_t>(model->mesh.indexCount),
                           currentFrame);

        // Poll Events
        pollEvents(&event);

        // Update last time
        lastTime = currentTime;
    }
    renderer.waitIdle();
    setScene(nullptr);
}

void vkx::Application::setScene(vkx::Scene *newScene) {
    scene.reset(newScene);
    if (newScene != nullptr) {
        // TODO this is a mess!
        scene->init(&config, this, renderer);
        mvpBuffers = renderer.createBuffers(vkx::MVP{});
        lightBuffers = renderer.createBuffers(vkx::DirectionalLight{});
        materialBuffers = renderer.createBuffers(vkx::Material{});
        renderer.createDescriptorSets(mvpBuffers, lightBuffers, materialBuffers, model->texture);
        scene->getViewport().setSize(config.windowWidth, config.windowHeight);
    }
}

void vkx::Application::pollEvents(SDL_Event *event) {
    // Poll events and compare them to event flags
    // Send them to appropriate functions that handle the events.
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_WINDOWEVENT:
                window->pollWindowEvent(event->window, scene.get());
                break;
            case SDL_KEYDOWN:
                handleKeyPressedEvent(event->key);
                break;
            case SDL_KEYUP:
                handleKeyReleasedEvent(event->key);
                break;
            case SDL_MOUSEMOTION:
                handleMouseMovedEvent(event->motion);
                break;
            default:
                return;
        }
    }
}

void vkx::Application::handleKeyPressedEvent(const SDL_KeyboardEvent &event) {
    scene->onKeyPress(event);
}

void vkx::Application::handleKeyReleasedEvent(const SDL_KeyboardEvent &event) {
    scene->onKeyRelease(event);
}

void vkx::Application::handleMouseMovedEvent(const SDL_MouseMotionEvent &event) {
    scene->onMouseMove(event);
}
