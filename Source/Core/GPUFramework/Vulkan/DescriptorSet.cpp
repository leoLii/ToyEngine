#include "DescriptorSet.hpp"

#include "Device.hpp"
#include "DescriptorSetLayout.hpp"
#include "Common/Logging.hpp"

DescriptorSet::DescriptorSet(
	const Device& device,
	DescriptorSetLayout* layout,
	vk::DescriptorPool pool,
	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos,
	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos)
	:device{ device }
	, layout{ layout }
	, pool{ pool }
	, bufferInfos{ bufferInfos }
	, imageInfos{ imageInfos }
{
	vk::DescriptorSetAllocateInfo allocateInfo;
	allocateInfo.descriptorPool = pool;
	allocateInfo.descriptorSetCount = 1;
	auto layoutHandle = layout->getHandle();
	allocateInfo.pSetLayouts = &layoutHandle;
	handle = device.getHandle().allocateDescriptorSets(allocateInfo).front();

	if (!writeDescriptorSets.empty())
	{
		LOGW("Trying to prepare a descriptor set that has already been prepared, skipping.");
		return;
	}

	for (auto& bindingInfo : bufferInfos)
	{
		auto  bindingIndex = bindingInfo.first;
		auto& bufferInfo = bindingInfo.second;
		auto layoutBinding = layout->getBindingInfo(bindingIndex);

		vk::WriteDescriptorSet writeDescriptorSet;
		writeDescriptorSet.dstBinding = bindingIndex;
		writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
		writeDescriptorSet.pBufferInfo = &bufferInfo;
		writeDescriptorSet.dstSet = handle;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorCount = 1;

		writeDescriptorSets.push_back(writeDescriptorSet);
	}

	// Iterate over all image bindings
	for (auto& bindingInfo : imageInfos)
	{
		auto  bindingIndex = bindingInfo.first;
		auto& imageInfo = bindingInfo.second;
		auto layoutBinding = layout->getBindingInfo(bindingIndex);

		vk::WriteDescriptorSet writeDescriptorSet;
		writeDescriptorSet.dstBinding = bindingIndex;
		writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
		writeDescriptorSet.pImageInfo = &imageInfo;
		writeDescriptorSet.dstSet = handle;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorCount = 1;

		writeDescriptorSets.push_back(writeDescriptorSet);
	}

	device.getHandle().updateDescriptorSets(writeDescriptorSets, nullptr);
}
