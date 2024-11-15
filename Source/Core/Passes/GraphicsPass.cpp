#include "GraphicsPass.hpp"

GraphicsPass::GraphicsPass(const GPUContext* context, ResourceManager* resourceManager, const Scene* scene)
	:gpuContext{ context }
	, resourceManager{ resourceManager }
	, scene{ scene }
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
