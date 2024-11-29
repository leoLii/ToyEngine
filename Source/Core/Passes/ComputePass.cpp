#include "ComputePass.hpp"

ComputePass::ComputePass(ResourceManager* resourceManager)
	:gpuContext{ GPUContext::GetInstance() }
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