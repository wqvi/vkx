#pragma once

constexpr static const std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace vkx {
    struct GlobalConfiguration;

    class Application;

    class SDLWindow;

    struct Scene;

    class SDLError;

    struct Node;

    class VulkanError;

    struct Profile;

    class RendererContext;

    class RendererBase;

    struct Vertex;

    template<class T>
    class Allocable;

    class Buffer;

    class BufferBase;

    class VertexBuffer;

    class IndexBuffer;

    class Image;

    template<class T>
    class UniformBuffer;

    class PhysicalDevice;

    class Device;

    class SingleTimeCommand;

    class DrawCommand;

    class Swapchain;

    class GraphicsPipeline;

    struct MVP;

    struct DirectionalLight;

    struct Material;

    class Mesh;

    class Texture;

    class Model;

    struct SyncObjects;
}