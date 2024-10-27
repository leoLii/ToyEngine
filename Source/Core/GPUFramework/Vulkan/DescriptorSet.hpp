#pragma once

#include "VkCommon.hpp"

#include <unordered_map>

class Device;
class DescriptorSetLayout;

class DescriptorSet {
public:
	DescriptorSet(
		const Device&,
		DescriptorSetLayout*,
		vk::DescriptorPool,
		std::unordered_map<uint32_t, vk::DescriptorBufferInfo>,
		std::unordered_map<uint32_t, vk::DescriptorImageInfo>);
	~DescriptorSet() = default;

	vk::DescriptorSet getHandle() {
		return handle;
	}

protected:
	const Device& device;

	DescriptorSetLayout* layout;

	vk::DescriptorPool pool;

	vk::DescriptorSet handle;

	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos;

	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets;

};