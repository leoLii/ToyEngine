#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;

class DescriptorSetLayout {
public:
	DescriptorSetLayout(
		const Device&, 
		uint32_t, 
		std::vector<vk::DescriptorSetLayoutBinding>&);
	~DescriptorSetLayout();

	vk::DescriptorSetLayout getHandle() const;

protected:
	const Device& device;
	
	vk::DescriptorSetLayout handle;

	const uint32_t setIndex;

	std::vector<vk::DescriptorSetLayoutBinding> bindings;

	std::vector<vk::DescriptorBindingFlags> bindingFlags;
};