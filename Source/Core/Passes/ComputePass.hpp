#pragma once

#include "Common/Math.hpp"

#include "Scene/Scene.hpp"

#include "Core/ResourceManager.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/ComputePipeline.hpp"


#include <vector>

class ComputePass {
public:
	ComputePass(const GPUContext*, ResourceManager*);
	virtual ~ComputePass();

	virtual void prepare() = 0;
	virtual void record(vk::CommandBuffer) = 0;
	virtual void update(uint32_t) = 0;

protected:
	const GPUContext* gpuContext;
	ResourceManager* resourceManager = nullptr;

	vk::PipelineCache pipelineCache;
	PipelineLayout* pipelineLayout = nullptr;
	ComputePipeline* computePipeline;

	vk::PushConstantRange constants;
	DescriptorSet* descriptorSet = nullptr;
	DescriptorSetLayout* descriptorSetLayout = nullptr;
};