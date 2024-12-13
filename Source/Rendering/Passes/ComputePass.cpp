#include "ComputePass.hpp"

ComputePass::ComputePass()
	:gpuContext{ GPUContext::GetSingleton() }
	, resourceManager{ ResourceManager::GetSingleton()}
{
}

ComputePass::~ComputePass()
{
	delete pipelineLayout;
	delete computePipeline;
	delete descriptorSet;
	delete descriptorSetLayout;
}