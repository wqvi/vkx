#pragma once

static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace vkx {
    class RendererContext;

    class RendererBase;

    struct Vertex;

    template<class T>
    class Allocable;

    class BufferBase;

    class VertexBuffer;

    class IndexBuffer;

    class Image;

    template<class T>
    class UniformBuffer;

    class CommandSubmitter;

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