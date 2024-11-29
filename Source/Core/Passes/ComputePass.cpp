#include "ComputePass.hpp"

ComputePass::ComputePass(const GPUContext* context, ResourceManager* resourceManager)
	:gpuContext{ context }
	, resourceManager{ resourceManager }
{
}

ComputePass::~ComputePass()
{
	delete pipelineLayout;
	delete computePipeline;
	delete descriptorSet;
	delete descriptorSetLayout;
}