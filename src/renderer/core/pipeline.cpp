#include <vkx/renderer/core/pipeline.hpp>

vkx::PipelineShader::PipelineShader(const Device &device, const std::string &file) {
    auto code = readFile(file);

    vk::ShaderModuleCreateInfo shaderCreateInfo{
            {},                                                  // flags
            static_cast<std::uint32_t>(code.size()),             // codeSize
            code.data() // pCode
    };

    auto module = device->createShaderModuleUnique(shaderCreateInfo);
}

std::vector<std::uint32_t> vkx::PipelineShader::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file.");
    }

    auto fileSize = file.tellg();
    std::vector<std::uint32_t> buffer(fileSize);

    file.seekg(std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    file.close();

    return buffer;
}

namespace vkx
{
    Pipeline::Pipeline(Device const &device, vk::UniqueDescriptorSetLayout const &descriptorSetLayout)
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            {},                  // flags
            *descriptorSetLayout // setLayouts
        };

        layout = device->createPipelineLayoutUnique(pipelineLayoutInfo);
    }

    std::vector<char> Pipeline::readFile(std::string const &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file.");
        }

        std::size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static std::vector<std::uint32_t> foobar(const std::string &filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file.");
        }

        auto fileSize = file.tellg();
        std::vector<std::uint32_t> buffer(fileSize);

        file.seekg(std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

        file.close();

        return buffer;
    }

    static vk::UniqueShaderModule barfoo(Device const &device, const std::vector<std::uint32_t> &data) {
        vk::ShaderModuleCreateInfo shaderCreateInfo{
                {},                                                  // flags
                static_cast<std::uint32_t>(data.size()),             // codeSize
                data.data() // pCode
        };

        return device->createShaderModuleUnique(shaderCreateInfo);
    }

    vk::UniqueShaderModule Pipeline::createShaderModule(Device const &device, std::vector<char> const &code)
    {
        vk::ShaderModuleCreateInfo shaderCreateInfo{
            {},                                                  // flags
            static_cast<std::uint32_t>(code.size()),             // codeSize
            reinterpret_cast<std::uint32_t const *>(code.data()) // pCode
        };

        return device->createShaderModuleUnique(shaderCreateInfo);
    }

    ComputePipeline::ComputePipeline(Device const &device, vk::UniqueDescriptorSetLayout const &descriptorSetLayout)
        : Pipeline(device, descriptorSetLayout)
    {
        auto computeShaderCode = Pipeline::readFile("shader.comp.spv");

        auto computeShaderModule = Pipeline::createShaderModule(device, computeShaderCode);

        vk::PipelineShaderStageCreateInfo shaderStage{
            {},                                // flags
            vk::ShaderStageFlagBits::eCompute, // stage
            *computeShaderModule,              // module
            "main"                             // pName
        };

        vk::ComputePipelineCreateInfo pipelineInfo{
            {},          // flags
            shaderStage, // stage
            *layout,     // layout
        };

        // Use Vulkan C functions to create a pipeline due to an odd difference in vulkan headers
        // Sometimes the api returns a pipeline value or sometimes returns a result value

        VkPipeline cPipeline = nullptr;
        if (vkCreateComputePipelines(static_cast<vk::Device>(device), nullptr, 1, reinterpret_cast<VkComputePipelineCreateInfo *>(&pipelineInfo), nullptr, &cPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline.");
        }

        pipeline = vk::UniquePipeline(cPipeline, *device);
    }

    GraphicsPipeline::GraphicsPipeline(Device const &device, vk::Extent2D const &extent, vk::UniqueRenderPass const &renderPass, vk::UniqueDescriptorSetLayout const &descriptorSetLayout)
        : Pipeline(device, descriptorSetLayout)
    {
        auto vertShaderCode = foobar("shader.vert.spv");
        auto fragShaderCode = foobar("shader.frag.spv");

        auto vertShaderModule = barfoo(device, vertShaderCode);
        auto fragShaderModule = barfoo(device, fragShaderCode);

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            {},                               // flags
            vk::ShaderStageFlagBits::eVertex, // stage
            *vertShaderModule,                // module
            "main"                            // pName
        };

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            {},                                 // flags
            vk::ShaderStageFlagBits::eFragment, // stage
            *fragShaderModule,                  // module
            "main"                              // pName
        };

        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
            vertShaderStageInfo,
            fragShaderStageInfo};

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            {},                   // flags
            bindingDescription,   // vertexBindingDescriptions
            attributeDescriptions // vertexAttributeDescriptions
        };

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
            {},                                   // flags
            vk::PrimitiveTopology::eTriangleList, // topology
            false                                 // primitiveRestartEnable
        };

        vk::Viewport viewport{
            0.0f,                              // x
            0.0f,                              // y
            static_cast<float>(extent.width),  // width
            static_cast<float>(extent.height), // height
            0.0f,                              // minDepth
            1.0f                               // maxDepth
        };

        vk::Rect2D scissor{
            {
                0, // x
                0  // y
            },     // offset
            extent // extent
        };

        vk::PipelineViewportStateCreateInfo viewportState{
            {},       // flags
            viewport, // viewports
            scissor   // scissors
        };

        vk::PipelineRasterizationStateCreateInfo rasterizer{
            {},                               // flags
            false,                            // depthClampEnable
            false,                            // rasterizerDiscardEnable
            vk::PolygonMode::eFill,           // polygonMode
            vk::CullModeFlagBits::eBack,      // cullMode
            vk::FrontFace::eCounterClockwise, // frontFace
            false,                            // depthBiasEnable
            {},                               // depthBiasConstantFactor
            {},                               // depthBiasClamp
            {},                               // depthBiasSlopeFactor
            1.0f                              // lineWidth
        };

        vk::PipelineMultisampleStateCreateInfo multisampling{
            {},                          // flags
            vk::SampleCountFlagBits::e1, // rasterizationSamples
            false                        // sampleShadingEnable
        };

        vk::PipelineDepthStencilStateCreateInfo depthStencil{
            {},                   // flags
            true,                 // depthTestEnable
            true,                 // depthWriteEnable
            vk::CompareOp::eLess, // depthCompareOp
            false,                // depthBoundsTestEnable
            false                 // stencilTestEnable
        };

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = false;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        std::array<float, 4> blendConstants{0.0f, 0.0f, 0.0f, 0.0f};
        vk::PipelineColorBlendStateCreateInfo colorBlending{
            {},                   // flags
            false,                // logicOpEnable
            vk::LogicOp::eCopy,   // logicOp
            colorBlendAttachment, // attachments
            blendConstants        // blendConstants
        };

        vk::GraphicsPipelineCreateInfo pipelineInfo{
            {},               // flags
            shaderStages,     // stages
            &vertexInputInfo, // pVertexInputInfo
            &inputAssembly,   // pInputAssemblyState
            {},               // pTessellationState
            &viewportState,   // pViewportState
            &rasterizer,      // pRasterizationState
            &multisampling,   // pMultisampleState
            &depthStencil,    // pDepthStencilState
            &colorBlending,   // pColorBlendState
            {},               // pDynamicState
            *layout,          // layout
            *renderPass,      // renderPass
            0,                // subpass
            nullptr           // basePipelineHandle
        };

        // Use Vulkan C functions to create a pipeline due to an odd difference in vulkan headers
        // Sometimes the api returns a pipeline value or sometimes returns a result value

        VkPipeline cPipeline = nullptr;
        if (vkCreateGraphicsPipelines(static_cast<vk::Device>(device), nullptr, 1, reinterpret_cast<VkGraphicsPipelineCreateInfo *>(&pipelineInfo), nullptr, &cPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline.");
        }

        pipeline = vk::UniquePipeline(cPipeline, *device);
    }
}