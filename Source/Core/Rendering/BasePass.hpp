#pragma once

#include "Common/Math.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Scene/Scene.hpp"

#include <vector>

struct Attachment {
	Image* image;
	ImageView* view;
	vk::Format format;
	vk::RenderingAttachmentInfo attachmentInfo;
};

class BasePass {
public:
	BasePass(const GPUContext*, const Scene*);
	~BasePass();

	void prepare();

	vk::CommandBuffer record();

	void end();

	vk::CommandBuffer getCommandBuffer() {
		return commandBuffer;
	}

protected:
	const GPUContext* gpuContext;
	const Scene* scene;

	vk::CommandBuffer commandBuffer;

	Attachment* colorAttachment;
	Attachment* depthAttachment;

	vk::PushConstantRange constants;
	GraphicsPipelineState* pipelineState;
	PipelineLayout* pipelineLayout;
	GraphicsPipeline* graphicsPipeline;
	DescriptorSet* descriptorSet;
	DescriptorSetLayout* descriptorSetLayout;
	vk::RenderingInfo renderingInfo;
	vk::Viewport viewport;
	vk::Rect2D scissor;

	Buffer* uniformBuffer;
	Buffer* vertexBuffer;
	Buffer* indexBuffer;
	Buffer* indirectDrawBuffer;
};