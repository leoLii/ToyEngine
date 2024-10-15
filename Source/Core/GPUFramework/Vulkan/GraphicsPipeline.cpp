#include "GraphicsPipeline.hpp"

#include "Device.hpp"
#include "ShaderModule.hpp"
#include "PipelineLayout.hpp"

#include <tuple>

GraphicsPipeline::GraphicsPipeline(
    const Device& device, 
    PipelineLayout& layout,
    GraphicsPipelineState& state, 
    std::vector<ShaderModule>& shaderModules)
    :device{ device }
    , state{ state }
{
    /*for (auto setLayout : layout.setLayouts) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        for (auto binding : setLayout.bindings) {
            vk::DescriptorSetLayoutBinding bindingInfo;
            bindingInfo.binding = binding.binding;
            bindingInfo.descriptorType = binding.descriptorType;
            bindingInfo.descriptorCount = binding.descriptorCount;
            bindingInfo.stageFlags = binding.shaderStage;
            bindingInfo.pImmutableSamplers = binding.immutableSamplers.data();
        }

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayout;
        descriptorSetLayout.bindingCount = bindings.size();
        descriptorSetLayout.pBindings = bindings.data();

        vk::DescriptorPoolCreateInfo info;
        info.

        device.getHandle().createDescriptorPool
    }

    for (auto constance : layout.constanceRanges) {
        vk::PushConstantRange range
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;*/

    //layout = device.getHandle().createPipelineLayout(pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = shaderModules[0].getHandle();
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = shaderModules[1].getHandle();
    fragShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo stages[] = {vertShaderStageInfo, fragShaderStageInfo};

    vk::VertexInputBindingDescription vertexInput;
    vertexInput.binding = 0;
    vertexInput.stride = 0;
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA; 
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicState;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::Format format = vk::Format::eB8G8R8A8Srgb;
    vk::PipelineRenderingCreateInfo renderingInfo;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &format;

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = shaderModules.size();
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout.getHandle();
    //pipelineInfo.renderPass = renderPass.getHandle();
    //pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pNext = &renderingInfo;

    vk::Result result;
    std::tie(result, handle) = device.getHandle().createGraphicsPipeline(nullptr, pipelineInfo);

    switch (result)
    {
        case vk::Result::eSuccess: break;
        case vk::Result::ePipelineCompileRequiredEXT:
        // something meaningfull here
            break;
        default: assert(false);  // should never happen
    }
}

GraphicsPipeline::~GraphicsPipeline() {
    device.getHandle().destroyPipeline(handle);
}

vk::Pipeline GraphicsPipeline::getHandle()
{
    return handle;
}
