#pragma once

#include "Common/Math.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/ResourceManager.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class GBufferPass {
public:
	GBufferPass(const GPUContext*, ResourceManager*, const Scene*, Vec2);
	~GBufferPass();

	void prepare();

	void record(vk::CommandBuffer);

	void update(uint32_t);

protected:
	struct alignas(16) Constant {
		Mat4 prevPV;
		Mat4 jitteredPV;
		Vec2 prevJitter;
		Vec2 currJitter;
	};

	struct alignas(16) Uniform {
		Mat4 prevModel;
		Mat4 currModel;
	};

	const GPUContext* gpuContext;
	ResourceManager* resourceManager;
	const Scene* scene;

	Attachment* positionAttachment;
	Attachment* albedoAttachment;
	Attachment* normalAttachment;
	Attachment* armAttachment;
	Attachment* velocityAttachment;
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

	uint32_t width;
	uint32_t height;

	std::vector<vk::RenderingAttachmentInfo> renderingAttachments;
	vk::RenderingAttachmentInfo depthAttachmentInfo;
	std::vector<vk::Format> attachmentFormats;

	std::vector<Uniform> uniforms;

private:
	void initAttachments();
};