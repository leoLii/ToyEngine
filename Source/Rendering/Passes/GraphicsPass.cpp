#include "GraphicsPass.hpp"

GraphicsPass::GraphicsPass()
	:gpuContext{ GPUContext::GetSingleton() }
	, resourceManager{ ResourceManager::GetSingleton() }
{
}

GraphicsPass::~GraphicsPass()
{
	delete pipelineState;
	delete pipelineLayout;
	delete graphicsPipeline;
	delete descriptorSet;
	delete descriptorSetLayout;
}
