#pragma once

#include "Common/Math.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class BasePass {
public:
	BasePass(const GPUContext*, const Scene*);
	~BasePass();

	void prepare();

	void record(vk::CommandBuffer);

	void update();

	const Image* getImage() const {
		return colorAttachment->image;
	}

protected:
	struct Constant {
		Mat4 prevPV;
		Mat4 jitteredPV;
	};

	struct Uniform {
		Mat4 prevModel;
		Mat4 currModel;
	};

	const GPUContext* gpuContext;
	const Scene* scene;

	Attachment* colorAttachment;
	Attachment* normalAttachment;
	Attachment* armAttachment;
	Attachment* motionAttachment;
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

	uint32_t width = 960;
	uint32_t height = 540;

	std::vector<vk::RenderingAttachmentInfo> renderingAttachments;
	std::vector<vk::Format> attachmentFormats;

	std::vector<Uniform> uniforms;

private:
	void initAttachments();
};