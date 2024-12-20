#include "ComputePipeline.hpp"

#include "Device.hpp"
#include "PipelineLayout.hpp"
#include "ShaderModule.hpp"

ComputePipeline::ComputePipeline(
	const Device& device,
	PipelineLayout* layout,
	vk::PipelineCache cache,
	const ShaderModule* shaderModule)
	:device { device } 
{
	vk::PipelineShaderStageCreateInfo stageInfo;
	stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
	stageInfo.module = shaderModule->getHandle();
	stageInfo.pName = "main";

	vk::ComputePipelineCreateInfo pipelineInfo;
	pipelineInfo.layout = layout->getHandle();
	pipelineInfo.stage = stageInfo;

	auto result = device.getHandle().createComputePipeline(cache, pipelineInfo);

	if (result.result == vk::Result::eSuccess) {
		handle = result.value;
	}
	else {
		std::runtime_error("Create Computepipeline failed!");
	}
}

ComputePipeline::~ComputePipeline()
{
	device.getHandle().destroyPipeline(handle);
}

vk::Pipeline ComputePipeline::getHandle() const {
	return handle;
}