#pragma once

#pragma once

#include "Common/Math.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class LightingPass {
public:
	LightingPass(const GPUContext*, const Scene*);
	~LightingPass();

	void prepare();

	void record(vk::CommandBuffer);

	void update(uint32_t);

	void setAttachment(uint32_t, Attachment*);

	Attachment* getAttachment();

protected:
	struct Constant {
		Vec3 cameraPosition;
	};

	struct Uniform {
		alignas(16) Vec3 lightColor;
		alignas(16) Vec3 lightDirection;
	};

	const GPUContext* gpuContext;
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

	uint32_t width = 960;
	uint32_t height = 540;

	std::vector<vk::RenderingAttachmentInfo> renderingAttachments;
	std::vector<vk::Format> attachmentFormats;

	Uniform uniform;

	vk::Sampler sampler;

	std::vector<uint32_t> indices;
	std::vector<float> vertices;

private:
	void initAttachments();
};