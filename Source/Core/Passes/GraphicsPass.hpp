#pragma once

#include "Common/Math.hpp"

#include "Scene/Scene.hpp"

#include "Core/ResourceManager.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/GraphicsPipeline.hpp"


#include <vector>

class GraphicsPass {
public:
	GraphicsPass(const GPUContext*, ResourceManager*, const Scene*, Vec2);
	virtual ~GraphicsPass();

	virtual void prepare() = 0;
	virtual void record(vk::CommandBuffer) = 0;
	virtual void update(uint32_t) = 0;

protected:
	const GPUContext* gpuContext;
	ResourceManager* resourceManager = nullptr;
	const Scene* scene;

	GraphicsPipelineState* pipelineState = nullptr;
	PipelineLayout* pipelineLayout = nullptr;
	GraphicsPipeline* graphicsPipeline = nullptr;

	vk::PushConstantRange constants;
	DescriptorSet* descriptorSet = nullptr;
	DescriptorSetLayout* descriptorSetLayout = nullptr;

	vk::RenderingInfo renderingInfo;
	vk::Viewport viewport;
	vk::Rect2D scissor;

	uint32_t width;
	uint32_t height;

	std::vector<vk::RenderingAttachmentInfo> renderingAttachments;
	std::vector<vk::Format> attachmentFormats;
};