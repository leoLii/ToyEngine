#pragma once

#include "VkCommon.hpp"

class Device;
class PipelineLayout;
class ShaderModule;

class ComputePipeline {
public:
	ComputePipeline() = delete;
	ComputePipeline(
		const Device&, 
		PipelineLayout*,
		vk::PipelineCache,
		const ShaderModule*);

	~ComputePipeline();

	vk::Pipeline getHandle() const;

protected:
	const Device& device;

	vk::Pipeline handle;
};