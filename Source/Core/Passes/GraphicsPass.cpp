#include "GraphicsPass.hpp"

GraphicsPass::GraphicsPass(ResourceManager* resourceManager)
	:gpuContext{ GPUContext::GetInstance() }
	, resourceManager{ resourceManager }
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
