#pragma once

#include "VkCommon.hpp"

#include <vector>
#include <unordered_map>

class Device;

class DescriptorSetLayout {
public:
	DescriptorSetLayout(
		const Device&, 
		uint32_t, 
		std::vector<vk::DescriptorSetLayoutBinding>);
	~DescriptorSetLayout();

	vk::DescriptorSetLayoutBinding getBindingInfo(uint32_t);

	vk::DescriptorSetLayout getHandle() const;

protected:
	const Device& device;
	
	vk::DescriptorSetLayout handle;

	const uint32_t setIndex;

	std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindingVector;
};