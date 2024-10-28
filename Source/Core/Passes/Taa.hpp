#pragma once

#include "Common/Math.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"

#include <vector>

class Scene;
class ComputePipeline;
class DescriptorSet;
class DescriptorSetLayout;

class TaaPass {
public:
	TaaPass(const GPUContext*, const Scene*);
	~TaaPass();

	void prepare();

	void update(uint32_t);

	void record(vk::CommandBuffer);

	void setAttachment(uint32_t, Attachment*);

	Attachment* getAttachment();

protected:
	struct Constant {
		Vec2 size;
	};

	const GPUContext* gpuContext;
	const Scene* scene;

	Attachment* lightingResult;
	Attachment* historyAttachment;
	Attachment* depthAttachment;
	Attachment* motionAttachment;

	vk::PushConstantRange constants;
	DescriptorSet* descriptorSet;
	DescriptorSetLayout* descriptorSetLayout;
	PipelineLayout* pipelineLayout;
	ComputePipeline* computePipeline;

	uint32_t width = 960;
	uint32_t height = 540;

private:
	void initAttachment();
};