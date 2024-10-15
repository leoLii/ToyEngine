#pragma once

#include "VkCommon.hpp"

class Device;

class PipelineLayout {
public:
	PipelineLayout(
		const Device&, 
		std::vector<vk::DescriptorSetLayout>&, 
		std::vector<vk::PushConstantRange>&);
	~PipelineLayout();

	vk::PipelineLayout getHandle() const;
protected:
	const Device& device;

	vk::PipelineLayout handle;

	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

	std::vector<vk::PushConstantRange> pushConstanceRanges;
};