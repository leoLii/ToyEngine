#include "ComputePass.hpp"

ComputePass::ComputePass(const GPUContext* context, ResourceManager* resourceManager, const Scene* scene)
	:gpuContext{ context }
	, resourceManager{ resourceManager }
	, scene{ scene }
{
}

ComputePass::~ComputePass()
{
	delete pipelineLayout;
	delete computePipeline;
	delete descriptorSet;
	delete descriptorSetLayout;
}