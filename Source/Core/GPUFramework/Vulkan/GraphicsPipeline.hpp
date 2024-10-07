#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;
class RenderPass;

class GraphicsPipeline {
public:
	GraphicsPipeline() = delete;
	GraphicsPipeline(const Device&, RenderPass&, std::vector<vk::ShaderModule>);

	~GraphicsPipeline();

	vk::Pipeline getHandle() {
		return this->handle;
	}


protected:
	const Device& device;

	RenderPass& renderPass;

	vk::Pipeline handle;

	std::vector<vk::ShaderModule> shaderModules;

	vk::PipelineLayout layout;

	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	
	std::vector<vk::ClearValue> clearValues;
};