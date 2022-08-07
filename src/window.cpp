#include <vkx/window.hpp>

#include <vkx/vkx_exceptions.hpp>

void vkx::SDLWindow::SDLDeleter::operator()(SDL_Window *ptr) const noexcept {
    if (ptr != nullptr) SDL_DestroyWindow(ptr);
}

vkx::SDLWindow::SDLWindow(char const *title, int width, int height) {
    // SDL must be initialized prior to creating this C window
    // Create with vulkan, resizable, and hidden flags
    SDL_Window *sdlWindow = SDL_CreateWindow(title,
                                             SDL_WINDOWPOS_UNDEFINED, // Let window manager deal with it
                                             SDL_WINDOWPOS_UNDEFINED,
                                             width,
                                             height,
                                             SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (sdlWindow == nullptr) { // SDL api states if the window pointer is null it is an error
        throw vkx::SDLError();
    }

    // Creates valid unique window wrapper
    // Does not use std::make_unique due to custom deleter
    cWindow = std::unique_ptr<SDL_Window, SDLDeleter>(sdlWindow);

    // Hides the cursor and locks it within the window
    // This should eventually only happen when in the first person camera
    setCursorRelative(true);
}

void vkx::SDLWindow::show() const noexcept {
    SDL_ShowWindow(cWindow.get());
}

void vkx::SDLWindow::hide() const noexcept {
    SDL_HideWindow(cWindow.get());
}

std::pair<int, int> vkx::SDLWindow::getSize() const noexcept {
    int width;
    int height;

    SDL_Vulkan_GetDrawableSize(cWindow.get(), &width, &height);

    return std::make_pair(width, height);
}

int vkx::SDLWindow::getWidth() const noexcept {
    int width;

    SDL_Vulkan_GetDrawableSize(cWindow.get(), &width, nullptr);

    return width;
}

int vkx::SDLWindow::getHeight() const noexcept {
    int height;

    SDL_Vulkan_GetDrawableSize(cWindow.get(), nullptr, &height);

    return height;
}

void vkx::SDLWindow::pollWindowEvent(const SDL_WindowEvent &event, vkx::Scene *scene) {
    // Since only window resize events are only polled a simple if statement is only necessary
    if (event.event == SDL_WINDOWEVENT_RESIZED) handleResizeEvent(event, scene);
}

void vkx::SDLWindow::handleResizeEvent(const SDL_WindowEvent &event, vkx::Scene *scene) {
    framebufferResized = true;
    // data1 x data2 is width x height
    scene->getViewport().setSize(event.data1, event.data2); // Resize viewport
    scene->onWindowResize(event.data1, event.data2);
}

std::vector<char const *> vkx::SDLWindow::getExtensions() const {
    std::uint32_t count = 0;

    // Query the amount of extensions
    if (SDL_Vulkan_GetInstanceExtensions(cWindow.get(), &count, nullptr) != SDL_TRUE) {
        throw vkx::SDLError();
    }

    // Allocate vector
    std::vector<char const *> extensions(count);

    // Query extensions
    if (SDL_Vulkan_GetInstanceExtensions(cWindow.get(), &count, extensions.data()) != SDL_TRUE) {
        throw vkx::SDLError();
    }

    // If built in debug add debug util extension
#ifdef DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

void vkx::SDLWindow::waitForEvents() const {
    // Wait until size is zero
    int width;
    int height;
    std::tie(width, height) = getSize();
    while (width == 0 || height == 0) {
        std::tie(width, height) = getSize();
        SDL_WaitEvent(nullptr);
    }
}

bool vkx::SDLWindow::isFramebufferResized() const noexcept {
    return framebufferResized;
}

void vkx::SDLWindow::setFramebufferResized(bool flag) noexcept {
    framebufferResized = flag;
}

void vkx::SDLWindow::setCursorRelative(bool relative) {
    int sdlErrorCode = SDL_SetRelativeMouseMode(static_cast<SDL_bool>(relative));
    if (sdlErrorCode < 0) { // SDL states that if the error is not zero it is an error
        throw vkx::SDLError();
    }
}
