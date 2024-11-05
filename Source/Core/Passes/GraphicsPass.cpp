#include "GraphicsPass.hpp"

GraphicsPass::GraphicsPass(const GPUContext* context, ResourceManager* resourceManager, const Scene* scene, Vec2 size)
	:gpuContext{ context }
	, resourceManager{ resourceManager }
	, scene{ scene }
	, width{uint32_t(size.x)}
, height{ uint32_t(size.y) }
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
