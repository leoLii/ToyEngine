#include "GraphicsPipeline.hpp"

#include "Device.hpp"
#include "ShaderModule.hpp"
#include "PipelineLayout.hpp"

#include <tuple>

GraphicsPipeline::GraphicsPipeline(
    const Device& device, 
    PipelineLayout& layout,
    GraphicsPipelineState& state, 
    std::vector<ShaderModule*>& shaderModules)
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
    vertShaderStageInfo.module = shaderModules[0]->getHandle();
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = shaderModules[1]->getHandle();
    fragShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo stages[] = {vertShaderStageInfo, fragShaderStageInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInput;
    vertexInput.vertexBindingDescriptionCount = state.vertexInputState.bindings.size();
    vertexInput.pVertexBindingDescriptions = state.vertexInputState.bindings.data();
    vertexInput.vertexAttributeDescriptionCount = state.vertexInputState.attributes.size();
    vertexInput.pVertexAttributeDescriptions = state.vertexInputState.attributes.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.topology = state.inputAssemblyState.topology;
    inputAssembly.primitiveRestartEnable = state.inputAssemblyState.primitive_restart_enable;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = state.viewportState.viewport_count;
    viewportState.scissorCount = state.viewportState.scissor_count;

    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.depthClampEnable = state.rasterizationState.depth_clamp_enable;
    rasterizer.rasterizerDiscardEnable = state.rasterizationState.rasterizer_discard_enable;
    rasterizer.polygonMode = state.rasterizationState.polygon_mode;
    rasterizer.lineWidth = state.rasterizationState.line_width;
    rasterizer.cullMode = state.rasterizationState.cull_mode;
    rasterizer.frontFace = state.rasterizationState.front_face;
    rasterizer.depthBiasEnable = state.rasterizationState.depth_bias_enable;

    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.sampleShadingEnable = state.multisampleState.sample_shading_enable;
    multisampling.rasterizationSamples = state.multisampleState.rasterization_samples;
    multisampling.alphaToCoverageEnable = state.multisampleState.alpha_to_coverage_enable;
    multisampling.alphaToOneEnable = state.multisampleState.alpha_to_one_enable;
    multisampling.minSampleShading = state.multisampleState.min_sample_shading;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = state.colorBlendAttachmentState.color_write_mask;
    colorBlendAttachment.blendEnable = state.colorBlendAttachmentState.blend_enable;
    colorBlendAttachment.colorBlendOp = state.colorBlendAttachmentState.color_blend_op;
    colorBlendAttachment.srcColorBlendFactor = state.colorBlendAttachmentState.src_color_blend_factor;
    colorBlendAttachment.dstColorBlendFactor = state.colorBlendAttachmentState.dst_color_blend_factor;
    colorBlendAttachment.alphaBlendOp = state.colorBlendAttachmentState.alpha_blend_op;
    colorBlendAttachment.srcAlphaBlendFactor = state.colorBlendAttachmentState.src_alpha_blend_factor;
    colorBlendAttachment.dstAlphaBlendFactor = state.colorBlendAttachmentState.dst_alpha_blend_factor;

    vk::PipelineColorBlendStateCreateInfo colorBlendingState;
    colorBlendingState.logicOpEnable = state.colorBlendState.logic_op_enable;
    colorBlendingState.logicOp = state.colorBlendState.logic_op;
    colorBlendingState.attachmentCount = 1;
    colorBlendingState.pAttachments = &colorBlendAttachment;
    colorBlendingState.blendConstants[0] = 0.0f;
    colorBlendingState.blendConstants[1] = 0.0f;
    colorBlendingState.blendConstants[2] = 0.0f;
    colorBlendingState.blendConstants[3] = 0.0f;

    vk::PipelineDynamicStateCreateInfo dynamicState;
    dynamicState.dynamicStateCount = state.dynamicStates.size();
    dynamicState.pDynamicStates = state.dynamicStates.data();

    vk::PipelineRenderingCreateInfo renderingInfo;
    renderingInfo.colorAttachmentCount = state.renderingInfo.colorAttachmentFormats.size();
    renderingInfo.pColorAttachmentFormats = state.renderingInfo.colorAttachmentFormats.data();

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = shaderModules.size();
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlendingState;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.layout = layout.getHandle();
    //pipelineInfo.renderPass = renderPass.getHandle();
    //pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

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
