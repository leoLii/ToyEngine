#pragma once

#pragma once

#include "Common/Math.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/ResourceManager.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class LightingPass {
public:
	LightingPass(const GPUContext*, ResourceManager*, const Scene*, Vec2);
	~LightingPass();

	void prepare();

	void record(vk::CommandBuffer);

	void update(uint32_t);

protected:
	struct Constant {
		Vec3 cameraPosition;
	};

	struct Uniform {
		alignas(16) Vec3 lightColor;
		alignas(16) Vec3 lightDirection;
	};

	const GPUContext* gpuContext;
	ResourceManager* resourceManager;
	const Scene* scene;

	Attachment* positionAttachment;
	Attachment* albedoAttachment;
	Attachment* normalAttachment;
	Attachment* armAttachment;

	Attachment* lightingAttachment;

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

	std::vector<vk::RenderingAttachmentInfo> renderingAttachments;
	std::vector<vk::Format> attachmentFormats;

	Uniform uniform;

	vk::Sampler sampler;

	std::vector<uint32_t> indices;
	std::vector<float> vertices;

	uint32_t width;
	uint32_t height;

private:
	void initAttachments();
};