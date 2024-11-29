#include "GraphicsPass.hpp"

GraphicsPass::GraphicsPass(const GPUContext* context, ResourceManager* resourceManager)
	:gpuContext{ context }
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
