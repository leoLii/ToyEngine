#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;

class GraphicsPipeline {
public:
	GraphicsPipeline() = delete;
	GraphicsPipeline(Device&);

	~GraphicsPipeline();

	vk::Pipeline getHandle() {
		return this->handle;
	}


protected:
	Device& device;

	vk::Pipeline handle;

	vk::PipelineLayout layout;

	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	
	std::vector<vk::ClearValue> clearValues;
};