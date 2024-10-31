#pragma once

#include "Common/Math.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/ResourceManager.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"

#include <vector>

class Scene;
class ComputePipeline;
class DescriptorSet;
class DescriptorSetLayout;

class TaaPass {
public:
	TaaPass(const GPUContext*, ResourceManager*, const Scene*, Vec2);
	~TaaPass();

	void prepare();

	void update(uint32_t);

	void record(vk::CommandBuffer);

	void end();

protected:
	struct Constant {
		Vec2 size;
		Vec2 jitter;
	};

	const GPUContext* gpuContext;
	ResourceManager* resourceManager;
	const Scene* scene;

	Attachment* taaOutput;
	Attachment* history;

	Attachment* lightingResult;
	Attachment* velocity;
	Attachment* depth;

	vk::PushConstantRange constants;
	DescriptorSet* descriptorSet;
	DescriptorSetLayout* descriptorSetLayout;
	PipelineLayout* pipelineLayout;
	ComputePipeline* computePipeline;
	vk::Sampler sampler1;
	vk::Sampler sampler2;
	uint32_t width;
	uint32_t height;

private:
	void initAttachment();
};